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

static uint32_t cmd_clock(int dd)
{
	uint32_t clock;
	uint16_t accuracy;

	if (hci_read_clock(dd, 0x00, 0x00, &clock, &accuracy, 1000) < 0)
	{
		perror("Reading clock failed");
		exit(1);
	}
	//printf("Clock:    0x%4.4x\n", btohl(clock));
	//printf("Accuracy: %.2f msec\n", (float)accuracy * 0.3125);
	// hci_close_dev(dd);
	return clock;
}

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	// -------------------------------- BEGIN NETWORK SOCKET SETUP -----------------------------------------------------
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

	// -------------------------------- BEGIN BLUETOOTH -----------------------------------------------------
	uint32_t bt_clock_trigger;
	uint32_t bt_clock_now;

	// Initialize connection to local bluetooth device
	int dd_local;
	int dev_id_local;
	dev_id_local = hci_get_route(NULL);
	dd_local = hci_open_dev(dev_id_local);
	if (dd_local < 0)
	{
		perror("HCI device open failed");
		exit(1);
	}

	uint32_t blink_delay = 5; // seconds of delay
	bt_clock_trigger = cmd_clock(dd_local);
	bt_clock_trigger += blink_delay * 3200;

	// send bt_clock over socket
	n = write(sockfd, &bt_clock_trigger, sizeof(bt_clock_trigger));
	if (n < 0)
		error("ERROR writing to socket");
	printf("bt_clock_trigger sent through socket %zu \n", bt_clock_trigger);
	printf("Now waiting for trigger event...\n");
	// Wait for time to pass
	bt_clock_now = cmd_clock(dd_local);
	while (bt_clock_now < bt_clock_trigger)
		bt_clock_now = cmd_clock(dd_local);

	digitalWrite(0, HIGH);
	sleep(1);
	digitalWrite(0, LOW);

	hci_close_dev(dd_local);
	close(sockfd);
	return 0;
}