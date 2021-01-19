/* The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <sys/time.h>
#include <time.h>
#include <mpg123.h>
#include <ao/ao.h>
#define BITS 8


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

// gcc -o bt_sync_node_red bt_sync_node_red.c -lbluetooth -lwiringPi -lmpg123 -lao -O2

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
	int error;
	mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;
	ao_device *dev;

    int driver;

    ao_sample_format format;
    int channels, encoding;
    long rate;
	
	printf("THIS PROGRAM WILL ONLY WORK IF ACCURATE CLOCK TIMES ARE SPECIFIED BY OOB. e.g. NODE-RED");

	if (argc < 3)

	{
		fprintf(stderr, "usage %s clk bt_adress\n", argv[0]);
		exit(0);
	}


	int n;
	uint32_t bt_clock_A_now;
	uint32_t bt_clock_B_now;
	uint32_t bt_offset;
	uint32_t bt_clock_trigger;
	int dd;
	int dev_id = -1;
	int dd_local;
	int dev_id_local;
	uint16_t handle;
	uint16_t accuracy;

	bt_clock_trigger = (uint32_t) atoi(argv[1]);

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
	bt_offset = bt_clock_A_now - bt_clock_B_now;
	printf("Calculated bluetooth clock offset: %zu \n", bt_offset);
	printf("Now waiting for trigger event...\n");

	// ---------------init audio player
	ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    /* open the file and get the decoding format */
    mpg123_open(mh, "/home/pi/sinatra_boots.mp3");
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);


	bt_clock_B_now = cmd_clock(dd_local) + bt_offset;
	// Wait until trigger time
	while (bt_clock_B_now < bt_clock_trigger)
	{
		bt_clock_B_now = cmd_clock(dd_local) + bt_offset;
	}
	//digitalWrite(0, HIGH);
	//sleep(1);
	//digitalWrite(0, LOW);
   /* decode and play */
   	off_t pos;
	off_t correct_pos;
    while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK){
        ao_play(dev, buffer, done);
		pos = mpg123_tell (mh);
		// IMPORTANT: Adjust current position in track in order to always be perfectly synced even if decoding speed is different
		correct_pos = (double) ((cmd_clock(dd_local)+ bt_offset) - bt_clock_trigger) / 3200.0 * rate; 
		/*printf("time passed: %f\n", (double) (cmd_clock(dd_local) - bt_clock_trigger) / 3200.0 );
		printf("rate: %li\n", rate);
		printf("pos: %li\n", pos);
		printf("correct_pos: %li\n", correct_pos);
		printf("off by: %li frames\n\n", correct_pos - pos);*/
		mpg123_seek(mh, correct_pos, SEEK_SET);

	}

    /* clean up */
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();

	hci_close_dev(dd);
	hci_close_dev(dd_local);
	return 0;
}