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

	// bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
	// bind() passes file descriptor, the address structure,
	// and the length of the address structure
	// This bind() call will bind  the socket to the current IP address on port, portno
	if (bind(sockfd, (struct sockaddr *)&serv_addr,
			 sizeof(serv_addr)) < 0)
		error("ERROR on binding");

	// This listen() call tells the socket to listen to the incoming connections.
	// The listen() function places all incoming connection into a backlog queue
	// until accept() call accepts the connection.
	// Here, we set the maximum size for the backlog queue to 5.
	listen(sockfd, 5);

	// The accept() call actually accepts an incoming connection
	clilen = sizeof(cli_addr);

	// This accept() function will write the connecting client's address info
	// into the the address structure and the size of that structure is clilen.
	// The accept() returns a new socket file descriptor for the accepted connection.
	// So, the original socket file descriptor can continue to be used
	// for accepting new connections while the new socker file descriptor is used for
	// communicating with the connected client.
	newsockfd = accept(sockfd,
					   (struct sockaddr *)&cli_addr, &clilen);
	if (newsockfd < 0)
		error("ERROR on accept");

	struct timespec cpu_clock_A;
	struct timespec cpu_clock_B;
	uint32_t bt_clock_A_now;
	uint32_t bt_clock_A;
	uint32_t bt_clock_B_now;
	uint32_t bt_offset;
	int dd;
	int dev_id = -1;
	int dd_local;
	int dev_id_local;
	uint16_t handle;
	uint16_t accuracy;

	// Read incoming bluetooth clock
	n = read(newsockfd, &bt_clock_A, sizeof(bt_clock_A));
	if (n < 0)
		error("ERROR reading from socket");

	// Read incoming CPU Clock (currently unused but may be useful for cpu clock synchronization but that has failed so far)
	n = read(newsockfd, &cpu_clock_A, sizeof(cpu_clock_A));
	if (n < 0)
		error("ERROR reading from socket");

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
	hci_close_dev(dd);*/

	// get bluetooth clock of local device
	bt_clock_B_now = cmd_clock(dd_local);
	bt_offset = bt_clock_A_now - bt_clock_B_now;

	printf("bluetooth clock offset %zu \n", bt_offset);

	clock_gettime(CLOCK_MONOTONIC_RAW, &cpu_clock_B);

	// for measuring clock drift
	uint32_t steps = 5;
	uint32_t tokio_drift[steps + 1];
	tokio_drift[0] = bt_offset;

	// bt ticks per sec ~3200
	uint32_t blink_delay = 5;
	uint32_t play_in = blink_delay;
	uint32_t b_now_test;
	for (int i = 0; i < steps; i++)
	{
		// Wait 2 seconds
		while (bt_clock_B_now < bt_clock_A + play_in * 3200)
		{
			bt_clock_B_now = cmd_clock(dd_local) + bt_offset;
		}

		digitalWrite(0, HIGH);
		sleep(1);
		digitalWrite(0, LOW);

		play_in += blink_delay;
		dev_id = -1;
		connect_bt(argv, &dd, &dev_id, &handle);
		//get clock A
		if (hci_read_clock(dd, handle, 0x01, &bt_clock_A_now, &accuracy, 1000) < 0)
		{
			perror("Reading clock failed");
			exit(1);
		}
		b_now_test = cmd_clock(dd_local);

		tokio_drift[i] = bt_clock_A_now - b_now_test;
	}

	hci_close_dev(dd);
	hci_close_dev(dd_local);
	close(newsockfd);
	close(sockfd);

	printf("Drift: \n [");
	for (int i = 0; i < steps; i++)
	{
		printf("%zu, ", tokio_drift[i]);
	}
	printf("] \n");

	return 0;
}