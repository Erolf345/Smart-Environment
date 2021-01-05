#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <wiringPi.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <signal.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define timespec_diff_macro(a, b, result)                \
	do                                                   \
	{                                                    \
		(result)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
		(result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
		if ((result)->tv_nsec < 0)                       \
		{                                                \
			--(result)->tv_sec;                          \
			(result)->tv_nsec += 1000000000;             \
		}                                                \
	} while (0)

//gcc -o bt_sync_A bt_sync_A.c -lbluetooth -lwiringPi

static int find_conn(int s, int dev_id, long arg)
{
	struct hci_conn_list_req *cl;
	struct hci_conn_info *ci;
	int i;
	if (!(cl = malloc(10 * sizeof(*ci) + sizeof(*cl))))
	{
		perror("Can't allocate memory");
		exit(1);
	}
	cl->dev_id = dev_id;
	cl->conn_num = 10;
	ci = cl->conn_info;
	if (ioctl(s, HCIGETCONNLIST, (void *)cl))
	{
		perror("Can't get connection list");
		exit(1);
	}
	for (i = 0; i < cl->conn_num; i++, ci++)
		if (!bacmp((bdaddr_t *)arg, &ci->bdaddr))
		{
			free(cl);
			return 1;
		}
	free(cl);
	return 0;
}

static uint32_t cmd_clock()
{
	int dev_id = -1;
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint8_t which;
	uint32_t handle, clock;
	uint16_t accuracy;
	int dd;

	bacpy(&bdaddr, BDADDR_ANY);

	if (dev_id < 0 && !bacmp(&bdaddr, BDADDR_ANY))
		dev_id = hci_get_route(NULL);
	if (dev_id < 0)
	{
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long)&bdaddr);
		if (dev_id < 0)
		{
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}
	dd = hci_open_dev(dev_id);
	if (dd < 0)
	{
		perror("HCI device open failed");
		exit(1);
	}
	if (bacmp(&bdaddr, BDADDR_ANY))
	{
		cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
		if (!cr)
		{
			perror("Can't allocate memory");
			exit(1);
		}
		bacpy(&cr->bdaddr, &bdaddr);
		cr->type = ACL_LINK;
		if (ioctl(dd, HCIGETCONNINFO, (unsigned long)cr) < 0)
		{
			perror("Get connection info failed");
			free(cr);
			exit(1);
		}
		handle = htobs(cr->conn_info->handle);
		which = 0x01;
		free(cr);
	}
	else
	{
		handle = 0x00;
		which = 0x00;
	}
	if (hci_read_clock(dd, handle, which, &clock, &accuracy, 1000) < 0)
	{
		perror("Reading clock failed");
		exit(1);
	}
	accuracy = btohs(accuracy);
	printf("Clock:    0x%4.4x\n", btohl(clock));
	printf("Accuracy: %.2f msec\n", (float)accuracy * 0.3125);
	hci_close_dev(dd);
	return clock;
}

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	wiringPiSetup();
	pinMode(0, OUTPUT);

	if (argc < 3)
	{
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(argv[1]);
	if (server == NULL)
	{
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		  (char *)&serv_addr.sin_addr.s_addr,
		  server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");

	struct timespec rawtime;

	// read my bt_clock
	uint32_t bt_clock;
	struct timespec cpu_clock;
	struct timespec diff;
	bt_clock = cmd_clock();
	clock_gettime(CLOCK_MONOTONIC_RAW, &cpu_clock);

	// send bt_clock
	n = write(sockfd, &bt_clock, sizeof(bt_clock));
	if (n < 0)
		error("ERROR writing to socket");

	// send cpu_clock
	n = write(sockfd, &cpu_clock, sizeof(cpu_clock));
	if (n < 0)
		error("ERROR writing to socket");

	cpu_clock.tv_sec += 2;

	clock_gettime(CLOCK_MONOTONIC_RAW, &rawtime);
	// blink at negotiated time
	while (TRUE)
	{
		clock_gettime(CLOCK_MONOTONIC_RAW, &rawtime);
		timespec_diff_macro(&cpu_clock, &rawtime, &diff);
		if (diff.tv_sec < 0 )
			break;
		// printf("Code time: %ld seconds, ", diff.tv_sec);
		// printf("%ld milliseconds                                          \r", (diff.tv_nsec));

	}
	
	digitalWrite(0, HIGH);
	sleep(1);
	digitalWrite(0, LOW);

	close(sockfd);
	return 0;
}