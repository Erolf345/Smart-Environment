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

// from <sys/time.h>
// used timersub macro, changed timeval to timespec
// kept the order of operands the same, that is a - b = result
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

#define timespec_add_macro(a, b, result)                 \
	do                                                   \
	{                                                    \
		(result)->tv_sec = (a)->tv_sec + (b)->tv_sec;    \
		(result)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec; \
		if ((result)->tv_nsec >= 1000000000)             \
		{                                                \
			++(result)->tv_sec;                          \
			(result)->tv_nsec -= 1000000000;             \
		}                                                \
	} while (0)

// gcc -o bt_sync_B bt_sync_B.c -lbluetooth -lwiringPi

static volatile int signal_received = 0;
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
	//printf("Clock:    0x%4.4x\n", btohl(clock));
	//printf("Accuracy: %.2f msec\n", (float)accuracy * 0.3125);
	hci_close_dev(dd);
	return clock;
}

static uint32_t cmd_clock_ext(int dev_id, int dd, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint8_t which;
	uint32_t handle, clock;
	uint16_t accuracy;

	str2ba(argv[2], &bdaddr);

	// if (dev_id < 0 && !bacmp(&bdaddr, BDADDR_ANY))
	// 	dev_id = hci_get_route(NULL);

	if (dev_id < 0)
	{
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long)&bdaddr);
		if (dev_id < 0)
		{
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	//dd = hci_open_dev(dev_id);
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
		//which = atoi(argv[2]);
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

	/*printf("Clock:    0x%4.4x\n", btohl(clock));
	printf("Accuracy: %.2f msec\n", (float)accuracy * 0.3125);*/

	//hci_close_dev(dd);
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
	struct timespec cpu_clock_A_current;
	struct timespec cpu_clock_B;
	struct timespec clock_offset;
	struct timespec diff;
	uint32_t bt_clock_A_now;
	uint32_t bt_clock_A;
	uint32_t bt_clock_B_now;
	uint32_t bt_offset;
	unsigned long long bt_ticks_passed;
	time_t seconds_passed;
	unsigned long long nanoseconds_passed;

	struct timespec raw_time;
	struct timespec start;

	n = read(newsockfd, &bt_clock_A, sizeof(bt_clock_A));
	if (n < 0)
		error("ERROR reading from socket");

	n = read(newsockfd, &cpu_clock_A, sizeof(cpu_clock_A));
	if (n < 0)
		error("ERROR reading from socket");

	// start clocks read
	int dd;
	int dev_id = -1;
	uint16_t handle;
	uint16_t accuracy;

	connect_bt(argv, &dd, &dev_id, &handle);

	if (hci_read_clock(dd, handle, 0x01, &bt_clock_A_now, &accuracy, 1000) < 0)
	{
		perror("Reading clock failed");
		exit(1);
	}
	
	bt_clock_B_now = cmd_clock();
	bt_offset = bt_clock_A_now - bt_clock_B_now; //TODO: this is not safe againsts overflos

	
	//bt_clock_A_now = cmd_clock_ext(dev_id, dd, argv);
	clock_gettime(CLOCK_MONOTONIC_RAW, &cpu_clock_B);

	while (bt_clock_A_now < bt_clock_A + 6400)
	{
	 	bt_clock_A_now = cmd_clock() + bt_offset;
	}

	digitalWrite(0, HIGH);
	sleep(1);
	digitalWrite(0, LOW);
	hci_close_dev(dd);
	close(newsockfd);
	close(sockfd);

	exit(0);

	// printf("Partner clock:    0x%4.4x\n", btohl(bt_clock_B + bt_offset));

	// each tick of the bluetooth clock = 313 ticks of the cpu clock
	// 1 second = 32.768 ticks
	bt_ticks_passed = bt_clock_A_now - bt_clock_A;

	//uint32_t bt_ticks_passed2 = bt_clock_A_now - bt_clock_A;
	//uint32_t bt_trigger_time = bt_clock_A_now + 2 * 32768 - (bt_ticks_passed2);

	//printf("%lu ticks passed (2)\n",bt_ticks_passed2);
	//printf("triggertime: %lu \n",bt_trigger_time);

	printf("%llu ticks passed!\n", bt_ticks_passed);

	seconds_passed = (time_t)(bt_ticks_passed / (uint32_t)32768); // here for completeness, should always be zero
	nanoseconds_passed = (bt_ticks_passed - ((unsigned long long)seconds_passed * 32768)) * 30517;
	printf("%ld seconds, ", seconds_passed);
	printf("%llu milliseconds passed since transmission\n", (nanoseconds_passed / 1000000));
	cpu_clock_A_current.tv_sec = cpu_clock_A.tv_sec + seconds_passed;
	nanoseconds_passed = cpu_clock_A.tv_nsec + nanoseconds_passed;
	if (nanoseconds_passed >= 1000000000)
	{
		++cpu_clock_A_current.tv_sec;
		nanoseconds_passed -= 1000000000;
	}
	cpu_clock_A_current.tv_nsec = cpu_clock_A.tv_nsec + (long)nanoseconds_passed;

	//timespec_diff_macro(&cpu_clock_A_current, &cpu_clock_B, &clock_offset);
	timespecsub(&cpu_clock_A_current, &cpu_clock_B, &clock_offset);

	printf("Clock offset: %ld seconds, ", clock_offset.tv_sec);
	printf("%ld milliseconds \n", (clock_offset.tv_nsec / 1000000));

	// blink 2 seconds after negotiated time
	cpu_clock_A.tv_sec += 2;

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);

	while (TRUE)
	{ //cpu_clock_A.tv_sec - (raw_time.tv_sec + seconds_offset) >= 0 || cpu_clock_A.tv_nsec - (raw_time.tv_nsec + nanos_offset) >= 0) // wait until trigger time
		clock_gettime(CLOCK_MONOTONIC_RAW, &raw_time);

		//timespec_add_macro(&raw_time, &clock_offset, &raw_time);
		timespecadd(&raw_time, &clock_offset, &raw_time);
		//timespec_diff_macro(&cpu_clock_A, &raw_time, &diff);
		timespecsub(&cpu_clock_A, &raw_time, &diff);
		//timespecsub(&cpu_clock_A_current, &cpu_clock_B, &clock_offset);

		if (diff.tv_sec <= 0 && diff.tv_nsec <= 5000000)
		{
			if (diff.tv_sec == 0)
			{
				printf("nanosecs: %ld\n", diff.tv_nsec);
			}
			break;
		}
		//printf("Code time: %ld seconds, ", diff.tv_sec);
		//printf("%ld milliseconds                                          \r", (diff.tv_nsec));
	}
	clock_gettime(CLOCK_MONOTONIC_RAW, &raw_time);
	timespec_diff_macro(&raw_time, &start, &diff);
	printf("Code time: %ld seconds, ", diff.tv_sec);
	printf("%ld milliseconds \n", (diff.tv_nsec / 1000000));

	// cpu_clock_A.tv_sec += 2;
	// clock_gettime(CLOCK_MONOTONIC_RAW, &raw_time);
	// // blink at negotiated time
	// while(cpu_clock_A.tv_sec - raw_time.tv_sec >= 0 || cpu_clock_A.tv_nsec >= raw_time.tv_nsec){ // wait until trigger time
	//     clock_gettime(CLOCK_MONOTONIC_RAW, &raw_time);
	// }

	bt_clock_A_now = cmd_clock_ext(dev_id, dd, argv);
	hci_close_dev(dd);

	bt_ticks_passed = bt_clock_A_now - bt_clock_A;
	double pass = bt_ticks_passed / 32768.0;
	printf("%f secs passed\n", pass);

	digitalWrite(0, HIGH);
	sleep(1);
	digitalWrite(0, LOW);

	close(newsockfd);
	close(sockfd);
	return 0;
}