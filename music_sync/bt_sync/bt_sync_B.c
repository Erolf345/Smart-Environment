/* The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <wiringPi.h>
//#include <sys/time.h>
#include <time.h>

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <signal.h>
#include <inttypes.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

//There is a timespecsub though I cannot use it smh
//it's the same
#define timespecadd(tsp, usp, vsp)                        \
	do                                                    \
	{                                                     \
		(vsp)->tv_sec = (tsp)->tv_sec + (usp)->tv_sec;    \
		(vsp)->tv_nsec = (tsp)->tv_nsec + (usp)->tv_nsec; \
		if ((vsp)->tv_nsec >= 1000000000L)                \
		{                                                 \
			(vsp)->tv_sec++;                              \
			(vsp)->tv_nsec -= 1000000000L;                \
		}                                                 \
	} while (0)
#define timespecsub(tsp, usp, vsp)                        \
	do                                                    \
	{                                                     \
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;    \
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec; \
		if ((vsp)->tv_nsec < 0)                           \
		{                                                 \
			(vsp)->tv_sec--;                              \
			(vsp)->tv_nsec += 1000000000L;                \
		}                                                 \
	} while (0)

// gcc -o bt_sync_B bt_sync_B.c -lbluetooth -lwiringPi

/*
* read bluetooth clock of given hci socket
*/
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
/*
* Connect to a given bluetooth device. This address will have to be paired already or been found in a recent scan
*/
static void connect_bt(char **argv, int *dd, int *dev_id, uint16_t *handle)
{
	bdaddr_t bdaddr;
	uint8_t role;
	unsigned int ptype;

	role = 0x01;
	ptype = HCI_DM1 | HCI_DM3 | HCI_DM5 | HCI_DH1 | HCI_DH3 | HCI_DH5;

	str2ba(argv[2], &bdaddr);

	if (*dev_id < 0)
	{
		*dev_id = hci_get_route(&bdaddr);
		if (*dev_id < 0)
		{
			fprintf(stderr, "Device is not available.\n");
			exit(1);
		}
	}

	*dd = hci_open_dev(*dev_id);
	if (*dd < 0)
	{
		perror("HCI device open failed");
		exit(1);
	}

	if (hci_create_connection(*dd, &bdaddr, htobs(ptype),
							  htobs(0x0000), role, handle, 25000) < 0)
		perror("Can't create connection");

	return;
}

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	wiringPiSetup();
	pinMode(0, OUTPUT);

	int sockfd, newsockfd, portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 3)
	{
		fprintf(stderr, "usage %s port bt_adress\n", argv[0]);
		exit(0);
	}

	// create a socket
	// socket(int domain, int type, int protocol)
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	// clear address structure
	bzero((char *)&serv_addr, sizeof(serv_addr));

	portno = atoi(argv[1]);

	/* setup the host_addr structure for use in bind call */
	// server byte order
	serv_addr.sin_family = AF_INET;

	// automatically be filled with current host's IP address
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	// convert short integer value for port must be converted into network byte order
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *)&serv_addr,
			 sizeof(serv_addr)) < 0)
		error("ERROR on binding");

	listen(sockfd, 5);

	clilen = sizeof(cli_addr);

	newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

	if (newsockfd < 0)
		error("ERROR on accept");

	uint32_t bt_clock_A_now;
	uint32_t bt_clock_trigger;
	uint32_t bt_clock_B_now;
	uint32_t bt_offset;
	int dd;
	int dev_id = -1;
	int dd_local;
	int dev_id_local;
	uint16_t handle;
	uint16_t accuracy;

	// Read incoming bluetooth clock
	n = read(newsockfd, &bt_clock_trigger, sizeof(bt_clock_trigger));
	if (n < 0)
		error("ERROR reading from socket");
	printf("bt_clock_trigger received: %zu \n", bt_clock_trigger);

	// Initialize dd and dev_id for calls to external bluetooth device
	connect_bt(argv, &dd, &dev_id, &handle);

	// Initialize connection to local bluetooth device
	dev_id_local = hci_get_route(NULL);
	dd_local = hci_open_dev(dev_id_local);
	if (dd_local < 0)
	{
		perror("HCI device open failed");
		exit(1);
	}

	// Get clock of external bluetooth device
	if (hci_read_clock(dd, handle, 0x01, &bt_clock_A_now, &accuracy, 1000) < 0)
	{
		perror("Reading clock failed");
		exit(1);
	}

	/*
	// Disconnect from bluetooth device
	if (hci_disconnect(dd, htobs(cr->conn_info->handle),
						reason, 10000) < 0)
		perror("Disconnect failed");

	free(cr);
	hci_close_dev(dd);
	*/

	// get bluetooth clock of local device
	bt_clock_B_now = cmd_clock(dd_local);

	// Calculate offset between the two clocks
	bt_offset = bt_clock_A_now - bt_clock_B_now;
	printf("Calculated bluetooth clock offset: %zu \n", bt_offset);
	printf("Clock A now: %zu \n", bt_clock_A_now);
	printf("Now waiting for trigger event...\n");
	bt_clock_A_now = cmd_clock(dd_local) + bt_offset;

	// Wait until trigger time
	while (bt_clock_A_now < bt_clock_trigger)// since we are highly time sensitive we use while instead of sleep
	{
		bt_clock_A_now = cmd_clock(dd_local) + bt_offset;
	}

	// BLINK
	digitalWrite(0, HIGH);
	sleep(1);
	digitalWrite(0, LOW);


	hci_close_dev(dd);
	hci_close_dev(dd_local);
	close(newsockfd);
	close(sockfd);

	return 0;
}
