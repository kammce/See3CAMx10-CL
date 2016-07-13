/*
 * Hidraw Userspace Example
 *
 * Copyright (c) 2010 Alan Ott <alan@signal11.us>
 * Copyright (c) 2010 Signal 11 Software
 *
 * The code may be used by anyone for any purpose,
 * and can serve as a starting point for developing
 * applications using hidraw.
 */

/* Linux */
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

/* Unix */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

const char *bus_str(int bus);

int main(int argc, char **argv)
{

	int zoom = 0;
	if(argc > 1) {
		zoom = atoi(argv[1]);
	}
	
	printf("argc = %d && zoom = %d\n", argc, zoom);

	int fd;
	int i, res, desc_size = 0;
	char buf[256];
	struct hidraw_report_descriptor rpt_desc;
	struct hidraw_devinfo info;
	char *device = "/dev/hidraw2";

/*	if (argc > 1){
		device = argv[1];
	}*/

	/* Open the Device with non-blocking reads. In real life,
	   don't use a hard coded path; use libudev instead. */
	fd = open(device, O_RDWR|O_NONBLOCK);

	if (fd < 0) {
		perror("Unable to open device");
		return 1;
	}

	memset(&rpt_desc, 0x0, sizeof(rpt_desc));
	memset(&info, 0x0, sizeof(info));
	memset(buf, 0x0, sizeof(buf));

	/* Get Report Descriptor Size */
	res = ioctl(fd, HIDIOCGRDESCSIZE, &desc_size);
	if (res < 0)
		perror("HIDIOCGRDESCSIZE");
	else
		printf("Report Descriptor Size: %d\n", desc_size);

	/* Get Report Descriptor */
	rpt_desc.size = desc_size;
	res = ioctl(fd, HIDIOCGRDESC, &rpt_desc);
	if (res < 0) {
		perror("HIDIOCGRDESC");
	} else {
		printf("Report Descriptor:\n");
		for (i = 0; i < rpt_desc.size; i++)
			printf("%hhx ", rpt_desc.value[i]);
		puts("\n");
	}

	/* Get Raw Name */
	res = ioctl(fd, HIDIOCGRAWNAME(256), buf);
	if (res < 0)
		perror("HIDIOCGRAWNAME");
	else
		printf("Raw Name: %s\n", buf);

	/* Get Physical Location */
	res = ioctl(fd, HIDIOCGRAWPHYS(256), buf);
	if (res < 0)
		perror("HIDIOCGRAWPHYS");
	else
		printf("Raw Phys: %s\n", buf);

	/* Get Raw Info */
	res = ioctl(fd, HIDIOCGRAWINFO, &info);
	if (res < 0) {
		perror("HIDIOCGRAWINFO");
	} else {
		printf("Raw Info:\n");
		printf("\tbustype: %d (%s)\n",
			info.bustype, bus_str(info.bustype));
		printf("\tvendor: 0x%04hx\n", info.vendor);
		printf("\tproduct: 0x%04hx\n", info.product);
	}

	/* Send a Report to the Device */
	/* Report Number */
	// buf[0] = 0x1; 
	// buf[1] = 0x77;
	// res = write(fd, buf, 2);
	// if (res < 0) {
	// 	printf("Error: %d\n", errno);
	// 	perror("write");
	// } else {
	// 	printf("write() wrote %d bytes\n", res);
	// }

	char CommandVal_hex[10] = "\0";
	uint8_t Command = SET_O_ZOOM;
	uint8_t Length = 2;
	uint8_t CommandValue = zoom;

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

	printf("ret = %d\n", ret);

	close(fd);
	return 0;
}

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
