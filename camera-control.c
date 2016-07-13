/* Linux */
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <libudev.h>

/* Unix */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* C */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define DEBUG 0

/*
 * Ugly hack to work around failing compilation on systems that don't
 * yet populate new version of hidraw.h to userspace.
 */
#ifndef HIDIOCSFEATURE
#warning Please have your distro update the userspace kernel headers
#define HIDIOCSFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x06, len)
#define HIDIOCGFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x07, len)
#endif

#define FX320_CAMERA_CONTROL	0x62
#define FX320_CAMERA_COMMAND	0x02
#define CAMERA_VID "2560"
#define CAMERA_PID "c034"

#define SET_CAPTURE			'C'
#define SET_TEMP_FILT		'D'
#define SET_WHITE_BAL		'F'
#define SET_CAM_RED			'G'
#define SET_CAM_BLUE		'H'
#define SET_CAM_ORIENTATION	'J'
#define SET_CONTRAST_BRIGHT	'K'
#define SET_SHUTTER_SPEED	'L'
#define SET_CAM_AF			'M'
#define SET_FM_INC			'N'
#define SET_FM_DEC			'O'
#define SET_O_ZOOM			'P'
#define SET_D_ZOOM			'Q'
#define SET_COMP_RATIO		'T'

#define BUFFER_LENGTH		65

typedef struct
{
	char NAME[40];
	char CMD;
	int LENGTH;
	int MIN;
	int MAX;
} Commands_t;

Commands_t commands[] = {
	{"optical-zoom", SET_O_ZOOM, 2, 0x00, 0x2B}, //0x2B = 43d
	{"digital-zoom", SET_D_ZOOM, 1, 0x00, 0x0F},
	//{"contrast-brightness", SET_CONTRAST_BRIGHT, 2, 0x00, 0xFF},
};

const char * bus_str(int bus)
{
	switch (bus) {
	case BUS_USB:
		return "USB";
		break;
	case BUS_HIL:
		return "HIL";
		break;
	case BUS_BLUETOOTH:
		return "Bluetooth";
		break;
	case BUS_VIRTUAL:
		return "Virtual";
		break;
	default:
		return "Other";
		break;
	}
}

void get_seecam(char cpath[]) {
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev;

	/* Create the udev object */
	udev = udev_new();
	if (!udev) {
		printf("Can't create udev\n");
		exit(1);
	}

	/* Create a list of the devices in the 'hidraw' subsystem. */
	enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "hidraw");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);
	/* For each item enumerated, print out its information.
	   udev_list_entry_foreach is a macro which expands to
	   a loop. The loop will be executed for each member in
	   devices, setting dev_list_entry to a list entry
	   which contains the device's path in /sys. */
	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *path;

		/* Get the filename of the /sys entry for the device
		   and create a udev_device object (dev) representing it */
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);

		/* usb_device_get_devnode() returns the path to the device node
		   itself in /dev. */
		const char * node = udev_device_get_devnode(dev);
		#if DEBUG
		printf("Device Node Path: %s\n", node);
		#endif

		/* The device pointed to by dev contains information about
		   the hidraw device. In order to get information about the
		   USB device, get the parent device with the
		   subsystem/devtype pair of "usb"/"usb_device". This will
		   be several levels up the tree, but the function will find
		   it.*/
		dev = udev_device_get_parent_with_subsystem_devtype(
		       dev,
		       "usb",
		       "usb_device");
		if (!dev) {
			printf("Unable to find parent usb device.");
			exit(1);
		}

		/* From here, we can call get_sysattr_value() for each file
		   in the device's /sys entry. The strings passed into these
		   functions (idProduct, idVendor, serial, etc.) correspond
		   directly to the files in the directory which represents
		   the USB device. Note that USB strings are Unicode, UCS2
		   encoded, but the strings returned from
		   udev_device_get_sysattr_value() are UTF-8 encoded. */
		if(strcmp(udev_device_get_sysattr_value(dev, "idVendor"), CAMERA_VID) == 0 &&
			strcmp(udev_device_get_sysattr_value(dev, "idProduct"), CAMERA_PID) == 0) {
			strcpy(cpath,node);
		}

		#if DEBUG
		printf("  VID/PID: %s %s\n",
		        udev_device_get_sysattr_value(dev,"idVendor"),
		        udev_device_get_sysattr_value(dev, "idProduct"));
		printf("  %s\n  %s\n",
		        udev_device_get_sysattr_value(dev,"manufacturer"),
		        udev_device_get_sysattr_value(dev,"product"));
		printf("  serial: %s\n",
		         udev_device_get_sysattr_value(dev, "serial"));
		#endif
		udev_device_unref(dev);
	}
	/* Free the enumerator object */
	udev_enumerate_unref(enumerate);

	udev_unref(udev);

	return;
}

void SendCameraCommand(char * device_path, uint8_t Command, uint8_t Length, uint8_t CommandValue) {

	int fd;
	int i, res, desc_size = 0;
	char buf[256];
	struct hidraw_report_descriptor rpt_desc;
	struct hidraw_devinfo info;
	char *device = device_path;

	/* Open the Device with non-blocking reads. In real life,
	   don't use a hard coded path; use libudev instead. */
	fd = open(device, O_RDWR|O_NONBLOCK);

	if (fd < 0) {
		perror("Unable to open device");
		return;
	}

	memset(&rpt_desc, 0x0, sizeof(rpt_desc));
	memset(&info, 0x0, sizeof(info));
	memset(buf, 0x0, sizeof(buf));

	/* Get Report Descriptor Size */
	res = ioctl(fd, HIDIOCGRDESCSIZE, &desc_size);
	if (res < 0) {
		perror("HIDIOCGRDESCSIZE");
	} else {
		#if DEBUG
		printf("Report Descriptor Size: %d\n", desc_size);
		#endif
	}

	/* Get Report Descriptor */
	rpt_desc.size = desc_size;
	res = ioctl(fd, HIDIOCGRDESC, &rpt_desc);
	if (res < 0) {
		perror("HIDIOCGRDESC");
	} else {
		#if DEBUG
		printf("Report Descriptor:\n");
		for (i = 0; i < rpt_desc.size; i++)
			printf("%hhx ", rpt_desc.value[i]);
		puts("\n");
		#endif
	}

	/* Get Raw Name */
	res = ioctl(fd, HIDIOCGRAWNAME(256), buf);
	if (res < 0) {
		perror("HIDIOCGRAWNAME");
	} else {
		#if DEBUG
		printf("Raw Name: %s\n", buf);
		#endif
	}

	/* Get Physical Location */
	res = ioctl(fd, HIDIOCGRAWPHYS(256), buf);
	if (res < 0) {
		perror("HIDIOCGRAWPHYS");
	} else {
		#if DEBUG
		printf("Raw Phys: %s\n", buf);
		#endif
	}

	/* Get Raw Info */
	res = ioctl(fd, HIDIOCGRAWINFO, &info);
	if (res < 0) {
		perror("HIDIOCGRAWINFO");
	} else {
		#if DEBUG
		printf("Raw Info:\n");
		printf("\tbustype: %d (%s)\n",
			info.bustype, bus_str(info.bustype));
		printf("\tvendor: 0x%04hx\n", info.vendor);
		printf("\tproduct: 0x%04hx\n", info.product);
		#endif
	}

	char CommandVal_hex[10] = "\0";

	sprintf(CommandVal_hex,"%X",CommandValue);

	//Set the Report Number
	//SET_O_ZOOM,2,30
	buf[0] = 0x00;
	buf[1] = FX320_CAMERA_CONTROL; 	/* Report Number */
	buf[2] = FX320_CAMERA_COMMAND; 	/* Report Number */

	buf[3] = (Length+1); 	/* length in hex */
	buf[4] = Command;	 	/* Command in ASCII */
	if (1 == Length) {
		/* Hex value of CommandValue in ASCII */
		buf[5] = CommandVal_hex[0];
	} else if (Length == 2) {
		if(CommandVal_hex[1] != 0x00) {
			/* Hex value of CommandValue in ASCII */
			buf[5] = CommandVal_hex[0];
			/* Hex value of CommandValue in ASCII */
			buf[6] = CommandVal_hex[1];
		} else {
			/* Pad extra zero in ASCII */
			buf[5] = 0x30;
		 	/* Hex value of CommandValue in ASCII */
			buf[6] = CommandVal_hex[0];
		}
	}
	/* Send a Report to the Device */
	int ret = write(fd, buf, BUFFER_LENGTH);

	#if DEBUG
	printf("ret = %d\n", ret);
	#endif

	close(fd);
	return;
}

const char help[] = "NAME\n"
"	   camera-control - Command line program to control See3X10Camera\n"
"\n"
"SYNOPSIS\n"
"	   camera-control [command] [value]\n"
"\n"
"OPTIONS\n"
"	   command : [min value - max value]\n"
"			optical-zoom : [0 - 43]\n"
"			digital-zoom : [0 - 15]\n";

int main (int argc, char **argv)
{
	char camera_hidraw_path[100] = {0};
	Commands_t selected_command;
	bool found = false;
	char command_value = 0;
	if(argc != 3) {
		printf(help);
		printf("%d", argc);
		return 0;
	}

	for (int i = 0; i < sizeof(commands)/sizeof(Commands_t); ++i)
	{
		if(strcmp(argv[1], commands[i].NAME) == 0) {
			selected_command = commands[i];
			command_value = atoi(argv[2]);
			found = true;
			break;
		}
	}
	if(!found) {
		printf(help);
		return 0;
	}

	get_seecam(camera_hidraw_path);
	#if DEBUG
	printf("\n\nCamera HIDRAW Device path is %s\n", camera_hidraw_path);
	printf("Command value = %d :: Command = %c\n", command_value, selected_command.CMD);
	#endif
	SendCameraCommand(camera_hidraw_path, selected_command.CMD, selected_command.LENGTH, command_value);
	return 0;
}
