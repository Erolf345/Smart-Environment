#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
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
#include <mpg123.h>
#include <ao/ao.h>

#define BITS 8

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

//gcc -o start_at_bt_clock start_at_bt_clock.c -lbluetooth -lwiringPi -lmpg123 -lao -O2


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

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	printf("PID:%i\n",getpid());
	fflush(stdout);
	mpg123_handle *mh;
    char *buffer;
    size_t buffer_size;
    size_t done;
    int err;
	ao_device *dev;

    int driver;

    ao_sample_format format;
    int channels, encoding;
    long rate;

	uint32_t bt_clock_trigger;
	uint32_t bt_clock_now;

	printf("THIS PROGRAM IS MEANT TO WORK IN CONJUNCTION WITH NODE-RED");

	bt_clock_trigger = (uint32_t) atoi(argv[1]);
	printf("wait until: %zu \n", bt_clock_trigger);

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
	// ---------------init audio player
	ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (char*) malloc(buffer_size * sizeof(unsigned char));

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


	// Wait for time to pass
	bt_clock_now = cmd_clock(dd_local);
	while (bt_clock_now < bt_clock_trigger)
		bt_clock_now = cmd_clock(dd_local);
	printf("clock_now: %zu \n", bt_clock_now);
	off_t pos;
	off_t correct_pos;

   /* decode and play */
    while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK){
        ao_play(dev, buffer, done);
		pos = mpg123_tell (mh);
		correct_pos = (double) (cmd_clock(dd_local) - bt_clock_trigger) / 3200.0 * rate;
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
	//system("mpg123 sinatra_boots.mp3");
	hci_close_dev(dd_local);
	return 0;
}
