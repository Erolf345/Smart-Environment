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
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>


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

static uint32_t cmd_clock()
{
    int dev_id = -1;
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint8_t which;
	uint32_t handle, clock;
	uint16_t accuracy;
	int opt, dd;

    bacpy(&bdaddr, BDADDR_ANY);

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
	hci_close_dev(dd);
    return clock;
}

static void cmd_clock_ext(int dev_id, char **argv)
{
	struct hci_conn_info_req *cr;
	bdaddr_t bdaddr;
	uint8_t which;
	uint32_t handle, clock;
	uint16_t accuracy;
	int opt, dd;


	str2ba(argv[0], &bdaddr);

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
		which = atoi(argv[1]);

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

	hci_close_dev(dd);
}

int main(int argc, char *argv[])
{
    uint32_t clock;
    clock = cmd_clock();
}