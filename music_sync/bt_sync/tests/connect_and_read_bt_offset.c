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



//gcc -o connect_test connect_and_read_bt_offset.c -lbluetooth


static volatile int signal_received = 0;
static int find_conn(int s, int dev_id, long arg)
{
	struct hci_conn_list_req *cl;
	struct hci_conn_info *ci;
	int i;

	if (!(cl = malloc(10 * sizeof(*ci) + sizeof(*cl)))) {
		perror("Can't allocate memory");
		exit(1);
	}
	cl->dev_id = dev_id;
	cl->conn_num = 10;
	ci = cl->conn_info;

	if (ioctl(s, HCIGETCONNLIST, (void *) cl)) {
		perror("Can't get connection list");
		exit(1);
	}

	for (i = 0; i < cl->conn_num; i++, ci++)
		if (!bacmp((bdaddr_t *) arg, &ci->bdaddr)) {
			free(cl);
			return 1;
		}

	free(cl);
	return 0;
}

static uint16_t cmd_clkoff(int dev_id, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint16_t offset;
	int opt, dd;

	str2ba(argv[1], &bdaddr);

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
	if (!cr) {
		perror("Can't allocate memory");
		exit(1);
	}

	bacpy(&cr->bdaddr, &bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
		perror("Get connection info failed");
		exit(1);
	}

	if (hci_read_clock_offset(dd, htobs(cr->conn_info->handle), &offset, 1000) < 0) {
		perror("Reading clock offset failed");
		exit(1);
	}

	printf("Clock offset: 0x%4.4x\n", btohs(offset));

	free(cr);

	hci_close_dev(dd);
	return offset;
}
static void cmd_clock(int dev_id, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint8_t which;
	uint32_t handle, clock;
	uint16_t accuracy;
	int dd;

	str2ba(argv[1], &bdaddr);

	if (dev_id < 0 && !bacmp(&bdaddr, BDADDR_ANY))
		dev_id = hci_get_route(NULL);

	if (dev_id < 0) {
		dev_id = hci_for_each_dev(HCI_UP, find_conn, (long) &bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Not connected.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	if (bacmp(&bdaddr, BDADDR_ANY)) {
		cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
		if (!cr) {
			perror("Can't allocate memory");
			exit(1);
		}

		bacpy(&cr->bdaddr, &bdaddr);
		cr->type = ACL_LINK;
		if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
			perror("Get connection info failed");
			free(cr);
			exit(1);
		}

		handle = htobs(cr->conn_info->handle);
		//which = atoi(argv[1]);
		which = 0x01;

		free(cr);
	} else {
		handle = 0x00;
		which = 0x00;
	}

	if (hci_read_clock(dd, handle, which, &clock, &accuracy, 1000) < 0) {
		perror("Reading clock failed");
		exit(1);
	}

	accuracy = btohs(accuracy);

	printf("Clock:    0x%4.4x\n", btohl(clock));
	printf("Accuracy: %.2f msec\n", (float) accuracy * 0.3125);


	if (hci_read_clock(dd, handle, which, &clock, &accuracy, 1000) < 0) {
		perror("Reading clock failed");
		exit(1);
	}

	accuracy = btohs(accuracy);

	printf("Clock:    0x%4.4x\n", btohl(clock));
	printf("Accuracy: %.2f msec\n", (float) accuracy * 0.3125);
	
	if (hci_read_clock(dd, handle, which, &clock, &accuracy, 1000) < 0) {
		perror("Reading clock failed");
		exit(1);
	}

	accuracy = btohs(accuracy);

	printf("Clock:    0x%4.4x\n", btohl(clock));
	printf("Accuracy: %.2f msec\n", (float) accuracy * 0.3125);
	
	if (hci_read_clock(dd, handle, which, &clock, &accuracy, 1000) < 0) {
		perror("Reading clock failed");
		exit(1);
	}

	accuracy = btohs(accuracy);

	printf("Clock:    0x%4.4x\n", btohl(clock));
	printf("Accuracy: %.2f msec\n", (float) accuracy * 0.3125);
	
	if (hci_read_clock(dd, handle, which, &clock, &accuracy, 1000) < 0) {
		perror("Reading clock failed");
		exit(1);
	}

	accuracy = btohs(accuracy);

	printf("Clock:    0x%4.4x\n", btohl(clock));
	printf("Accuracy: %.2f msec\n", (float) accuracy * 0.3125);
	
	if (hci_read_clock(dd, handle, which, &clock, &accuracy, 1000) < 0) {
		perror("Reading clock failed");
		exit(1);
	}

	accuracy = btohs(accuracy);

	printf("Clock:    0x%4.4x\n", btohl(clock));
	printf("Accuracy: %.2f msec\n", (float) accuracy * 0.3125);
	
	if (hci_read_clock(dd, handle, which, &clock, &accuracy, 1000) < 0) {
		perror("Reading clock failed");
		exit(1);
	}

	accuracy = btohs(accuracy);

	printf("Clock:    0x%4.4x\n", btohl(clock));
	printf("Accuracy: %.2f msec\n", (float) accuracy * 0.3125);
	hci_close_dev(dd);
}


static void cmd_cc(char **argv)
{
	bdaddr_t bdaddr;
	uint16_t handle;
	uint8_t role;
	unsigned int ptype;
	int dd, opt;
	int dev_id = -1;

	role = 0x01;
	ptype = HCI_DM1 | HCI_DM3 | HCI_DM5 | HCI_DH1 | HCI_DH3 | HCI_DH5;

	str2ba(argv[1], &bdaddr);

	if (dev_id < 0) {
		dev_id = hci_get_route(&bdaddr);
		if (dev_id < 0) {
			fprintf(stderr, "Device is not available.\n");
			exit(1);
		}
	}

	dd = hci_open_dev(dev_id);
	if (dd < 0) {
		perror("HCI device open failed");
		exit(1);
	}

	if (hci_create_connection(dd, &bdaddr, htobs(ptype),
				htobs(0x0000), role, &handle, 25000) < 0)
		perror("Can't create connection");

	//uint16_t clk_off = cmd_clkoff(dev_id, argv);
	cmd_clock(dev_id, argv);


	hci_close_dev(dd);
	return;
}


int main(int argc, char *argv[])
{
	uint16_t bt_offset;

	cmd_cc(argv);
	
	return 0;
}