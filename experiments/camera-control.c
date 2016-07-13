//gcc -Wall -g -o camera-control camera-control.c -ludev

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>      
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <libudev.h>
#include <locale.h>
#include <unistd.h>

#define FX320_CAMERA_CONTROL	0x62
#define FX320_CAMERA_COMMAND	0x02

#define SEE3CAM_USB_VID		0x2560
#define SEE3CAM_10CUG_M_PID	0xc110
#define SEE3CAM_10CUG_C_PID	0xc111
#define SEE3CAM_80USB_PID	0xc080
#define SEE3CAM_11CUG_C_PID	0xc112
#define SEE3CAM_30_Z10X_PID	0xc034
#define SEE3CAM_CU50_PID	0xc151

#define APPLICATION_READY 	0x12
#define READFIRMWAREVERSION	0x40
#define ENABLEMASTERMODE	0x50
#define ENABLETRIGGERMODE	0x51
#define ENABLE_CROPPED_VGA_MODE	0x52
#define ENABLE_BINNED_VGA_MODE	0x53

#define OS_CODE			0x70
#define LINUX_OS		0x01
#define CAPTURE_COMPLETE	0x71

#define CAMERA_CONTROL_80	0x60
#define CAMERA_CONTROL_50	0x64
#define GET_FOCUS_MODE		0x01
#define SET_FOCUS_MODE		0x02
#define GET_FOCUS_POSITION	0x03
#define SET_FOCUS_POSITION	0x04
#define GET_FOCUS_STATUS	0x05
#define GET_FLASH_LEVEL		0x06
#define GET_TORCH_LEVEL		0x07
#define SET_FLASH_LEVEL		0x08
#define SET_TORCH_LEVEL		0x09

#define CONTINOUS_FOCUS		0x01
#define MANUAL_FOCUS		0x02
#define SINGLE_TRIG_FOCUS	0x03

#define FOCUS_FAILED		0x00
#define FOCUS_SUCCEEDED		0x01
#define FOCUS_BUSY		0x02

#define FLASH_OFF		0x00
#define FLASH_ON		0x01

#define SET_FAIL		0x00
#define SET_SUCCESS		0x01

#define GPIO_OPERATION		0x20
#define GPIO_GET_LEVEL		0x01
#define GPIO_SET_LEVEL		0x02

#define GPIO_LOW		0x00
#define GPIO_HIGH		0x01
#define GPIO_LEVEL_FAIL		0x00
#define GPIO_LEVEL_SUCCESS	0x01

#define WHITE_BAL_CONTROL	0x61
#define GET_WB_MODE		0x01
#define SET_WB_MODE		0x02
#define GET_WB_GAIN		0x03
#define SET_WB_GAIN		0x04
#define SET_WB_DEFAULTS		0x05
#define SET_ALL_WB_GAINS	0x06

#define WB_AUTO			0x01
#define WB_MANUAL		0x02

#define WB_RED			0x01
#define WB_GREEN		0x02
#define WB_BLUE			0x03

#define WB_FAIL			0x00
#define WB_SUCCESS		0x01

//See3CAM_Z10X defines
#define FX320_CAMERA_CONTROL	0x62
#define FX320_CAMERA_COMMAND	0x02
#define FX320_FIRMWARE_VERSION	0x03

#define CARRIAGE_RETURN		0x0d

#define SET_CAPTURE		'C'
#define SET_TEMP_FILT		'D'
#define SET_WHITE_BAL		'F'
#define SET_CAM_RED		'G'
#define SET_CAM_BLUE		'H'
#define SET_CAM_ORIENTATION	'J'
#define SET_CONTRAST_BRIGHT	'K'
#define SET_SHUTTER_SPEED	'L'
#define SET_CAM_AF		'M'
#define SET_FM_INC		'N'
#define SET_FM_DEC		'O'
#define SET_O_ZOOM		'P'
#define SET_D_ZOOM		'Q'
#define SET_COMP_RATIO		'T'

#define FX320_RESET		'0'

#define BUFFER_LENGTH		65
#define TIMEOUT			2000
#define FX320_TIMEOUT		10000
#define FX320_RESET_TIMEOUT	15000

#define FX320_ACK		0x01
#define FX320_NACK		0x02
#define FX320_T_OUT		0x03

#define SUCCESS			1
#define FAILURE			-1

#define DEVICE_NAME_MAX				32		// Example: /dev/hidraw0
#define MANUFACTURER_NAME_MAX			64		// Example: e-con Systems
#define PRODUCT_NAME_MAX			128		// Example: e-con's 1MP Monochrome Camera
#define HID_LIST_MAX				32

enum see3cam_device_index
{
	SEE3CAM_10CUG = 1,
	SEE3CAM_11CUG,
	SEE3CAM_80USB,
	SEE3CAM_30_Z10X,
	SEE3CAM_CU50,
};

extern int hid_fd;

unsigned char g_out_packet_buf[BUFFER_LENGTH];
unsigned char g_in_packet_buf[BUFFER_LENGTH];

const char	*hid_device;

uint8_t curr_see3cam_dev=0;
int hid_fd = -1;

/*
  **********************************************************************************************************
 *  MODULE TYPE	:	Internal API 									    *
 *  Name	:	GetTicketCount									    *
 *  Parameter1	:											    *
 *  Parameter2	:											    *
 *  Returns	:	unsigned int									    *
 *  Description	:       to return current time in the milli seconds				 	    *	
  **********************************************************************************************************
*/

unsigned int GetTickCount()
{
        struct timeval tv;
        if(gettimeofday(&tv, NULL) != 0)
                return 0;

        return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

/*
  **********************************************************************************************************
 *  MODULE TYPE	:	LIBRAY API 									    *
 *  Name	:	find_hid_device									    *
 *  Parameter1	:											    *
 *  Parameter2	:											    *
 *  Returns	:	int (SUCCESS or FAILURE)							    *
 *  Description	:       to find the first e-con's hid device connected to the linux pc		 	    *	
  **********************************************************************************************************
*/


int find_hid_device(char *videobusname)
{
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev, *pdev;
	int ret = FAILURE;
	char buf[256];
	
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
	
	/* For each item enumerated, print out its information. udev_list_entry_foreach is a macro which expands to a loop. The loop will be executed for each member in
	   devices, setting dev_list_entry to a list entry which contains the device's path in /sys. */
	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *path;
		
		/* Get the filename of the /sys entry for the device and create a udev_device object (dev) representing it */
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);

		/* usb_device_get_devnode() returns the path to the device node itself in /dev. */
		//printf("Device Node Path: %s\n", udev_device_get_devnode(dev));
		
		/* The device pointed to by dev contains information about the hidraw device. In order to get information about the USB device, get the parent device with the subsystem/devtype pair of "usb"/"usb_device". This will be several levels up the tree, but the function will find it.*/
		pdev = udev_device_get_parent_with_subsystem_devtype(
		       dev,
		       "usb",
		       "usb_device");
		if (!pdev) {
			printf("Unable to find parent usb device.");
			exit(1);
		}
	
		/* From here, we can call get_sysattr_value() for each file in the device's /sys entry. The strings passed into these functions (idProduct, idVendor, serial, 			etc.) correspond directly to the files in the /sys directory which represents the USB device. Note that USB strings are Unicode, UCS2 encoded, but the strings    		returned from udev_device_get_sysattr_value() are UTF-8 encoded. */
		/*printf("  VID/PID: %s %s\n",
		        udev_device_get_sysattr_value(pdev,"idVendor"),
		        udev_device_get_sysattr_value(pdev, "idProduct"));
		printf("  %s\n  %s\n",
		        udev_device_get_sysattr_value(pdev,"manufacturer"),
		        udev_device_get_sysattr_value(pdev,"product"));
		printf("  serial: %s\n",
		         udev_device_get_sysattr_value(pdev, "serial"));*/

		if(!strncmp(udev_device_get_sysattr_value(pdev,"idVendor"), "2560", 4)) {
			switch(curr_see3cam_dev) {
			case SEE3CAM_10CUG :
				if(!strncmp(udev_device_get_sysattr_value(pdev, "idProduct"), "c110", 4) || (!strncmp(udev_device_get_sysattr_value(pdev, "idProduct"), "c111", 4)) ) { 
					hid_device = udev_device_get_devnode(dev);
					udev_device_unref(pdev);
				}
					break;			
			case SEE3CAM_11CUG :
				if(!strncmp(udev_device_get_sysattr_value(pdev, "idProduct"), "c112", 4)) {
					hid_device = udev_device_get_devnode(dev);
					udev_device_unref(pdev);
				}
					break;			
			case SEE3CAM_80USB :
				if(!strncmp(udev_device_get_sysattr_value(pdev, "idProduct"), "c080", 4)) {
					hid_device = udev_device_get_devnode(dev);
					udev_device_unref(pdev);
				}
					break;			
			case SEE3CAM_30_Z10X :
				if(!strncmp(udev_device_get_sysattr_value(pdev, "idProduct"), "c034", 4)) {
					hid_device = udev_device_get_devnode(dev);
					udev_device_unref(pdev);
				}
					break;			
			case SEE3CAM_CU50 :
				if(!strncmp(udev_device_get_sysattr_value(pdev, "idProduct"), "c151", 4)) {
					hid_device = udev_device_get_devnode(dev);
					udev_device_unref(pdev);
				}
					break;			
			}
		}
		//Open each hid device and Check for bus name here
		hid_fd = open(hid_device, O_RDWR|O_NONBLOCK);

		if (hid_fd < 0) {
			perror("Unable to open device");
			continue;
		}else
			memset(buf, 0x00, sizeof(buf));

		/* Get Physical Location */
		ret = ioctl(hid_fd, HIDIOCGRAWPHYS(256), buf);
		if (ret < 0) {
			perror("HIDIOCGRAWPHYS");
		}
		//check if bus names are same or else close the hid device
		if(!strncmp(videobusname,buf,strlen(videobusname))){
			ret = SUCCESS;
		}
		/* Close the hid fd */
		if(hid_fd > 0)
		{
			if(close(hid_fd) < 0) {
				printf("\nFailed to close %s\n",hid_device);
			}
		}
		if(ret == SUCCESS) {
			return true;
		}
	}
/* Free the enumerator object */
	udev_enumerate_unref(enumerate);

	udev_unref(udev);

	return ret;
}


/*
  **********************************************************************************************************
 *  MODULE TYPE	:	LIBRAY API 									    *
 *  Name	:	SendOSCode									    *
 *  Parameter1	:											    *
 *  Parameter2	:											    *
 *  Returns	:	bool (true or false)								    *
 *  Description	:       Sends the camera command code for Linux OS identification		 	    *
  **********************************************************************************************************
*/
bool SendOSCode()
{
	int ret = 0;
	bool timeout = true;
	unsigned int start, end = 0;

	//Initialize the buffer
	memset(g_out_packet_buf, 0x00, sizeof(g_out_packet_buf));

	//Set the Report Number for OS identification
	g_out_packet_buf[1] = OS_CODE; 	/* Report Number OS identification */
	g_out_packet_buf[2] = LINUX_OS;	/* Report Number for Linux OS */

	/* Send a Report to the Device */
	ret = write(hid_fd, g_out_packet_buf, BUFFER_LENGTH);
	if (ret < 0) {
		perror("write");
		printf("\nOS Identification Failed\n");
		return false;
	} else {
		printf("%s(): wrote %d bytes\n", __func__,ret);
		printf("\nSent Linux OS identificaton code\n");
	}
	start = GetTickCount();
	while(timeout) 
	{	
		/* Get a report from the device */
		ret = read(hid_fd, g_in_packet_buf, BUFFER_LENGTH);
		if (ret < 0) {
			//perror("read");
		} else {
			printf("%s(): read %d bytes:\n", __func__,ret);
			if(g_in_packet_buf[0] == OS_CODE &&
				g_in_packet_buf[1] == LINUX_OS ) {
					if(g_in_packet_buf[2] == SET_SUCCESS)
						printf("\nSet Success\n");
					else if (g_in_packet_buf[2] == SET_FAIL){
						printf("\nSet Failed\n");
						return false;
					}
					else {
						printf("\nUnknown return value\n");
						return false;
					}
					timeout = false;
			}
		}
		end = GetTickCount();
		if(end - start > TIMEOUT)
		{
			printf("%s(): Timeout occurred\n", __func__);
			printf("\nOS Identification Failed\n");
			timeout = false;
			return false;
		}
	}
	return true;
}

/*
  **********************************************************************************************************
 *  MODULE TYPE	:	LIBRAY API 									    *
 *  Name	:	InitExtensionUnit								    *
 *  Parameter1	:											    *
 *  Parameter2	:											    *
 *  Returns	:	bool (true or false)								    *
 *  Description	:       finds see3cam device based on the VID, PID and busname and initialize the HID	    *
			device. Application should call this function before calling any other function.    *
  **********************************************************************************************************
*/

bool InitExtensionUnit(uint8_t *busname)
{
	int i, ret, desc_size = 0;
	char buf[256];
	struct hidraw_devinfo info;
	struct hidraw_report_descriptor rpt_desc;
	ret = find_hid_device((char*)busname);
	if(ret < 0)
	{
		printf("%s(): Not able to find the e-con's see3cam device\n", __func__);
		return false;
	}
	printf(" Selected HID Device : %s\n",hid_device);

	/* Open the Device with non-blocking reads. In real life,
	   don't use a hard coded path; use libudev instead. */
	hid_fd = open(hid_device, O_RDWR|O_NONBLOCK);

	if (hid_fd < 0) {
		perror("Unable to open device");
		return false;
	}

	memset(&rpt_desc, 0x0, sizeof(rpt_desc));
	memset(&info, 0x0, sizeof(info));
	memset(buf, 0x0, sizeof(buf));

	/* Get Report Descriptor Size */
	ret = ioctl(hid_fd, HIDIOCGRDESCSIZE, &desc_size);
	if (ret < 0) {
		perror("HIDIOCGRDESCSIZE");
		return false;
	}
	else
		printf("Report Descriptor Size: %d\n", desc_size);

	/* Get Report Descriptor */
	rpt_desc.size = desc_size;
	ret = ioctl(hid_fd, HIDIOCGRDESC, &rpt_desc);
	if (ret < 0) {
		perror("HIDIOCGRDESC");
		return false;
	} else {
		printf("Report Descriptors:\n");
		for (i = 0; i < rpt_desc.size; i++)
			printf("%hhx ", rpt_desc.value[i]);
		puts("\n");
	}

	/* Get Raw Name */
	ret = ioctl(hid_fd, HIDIOCGRAWNAME(256), buf);
	if (ret < 0) {
		perror("HIDIOCGRAWNAME");
		return false;
	}
	else
		printf("Raw Name: %s\n", buf);

	/* Get Physical Location */
	ret = ioctl(hid_fd, HIDIOCGRAWPHYS(256), buf);
	if (ret < 0) {
		perror("HIDIOCGRAWPHYS");
		return false;
	}
	else
		printf("Raw Phys: %s\n", buf);

	/* Get Raw Info */
	ret = ioctl(hid_fd, HIDIOCGRAWINFO, &info);
	if (ret < 0) {
		perror("HIDIOCGRAWINFO");
		return false;
	} else {
		printf("Raw Info:\n");
		//printf("\tbustype: %d (%s)\n", info.bustype, bus_str(info.bustype));
		printf("\tvendor: 0x%04hx\n", info.vendor);
		printf("\tproduct: 0x%04hx\n", info.product);
	}
	
#if 1
	if(curr_see3cam_dev == SEE3CAM_11CUG || 
		curr_see3cam_dev == SEE3CAM_10CUG ||
		curr_see3cam_dev == SEE3CAM_CU50 ||
		curr_see3cam_dev == SEE3CAM_80USB ) {
			ret=SendOSCode();
		if (ret == false) {
			printf("OS Identification failed\n");
//			return false;
		}
	} else {
		printf("\nOS identification not supported by camera\n");
	}
#endif
	return true;
}

/*
  **********************************************************************************************************
 *  MODULE TYPE	:	LIBRAY API 									    *
 *  Name	:	ReadFirmwareVersion								    *
 *  Parameter1	:	unsigned char *		(Major Version)						    *
 *  Parameter2	:	unsigned char *		(Minor Version 1)					    *
 *  Parameter3	:	unsigned short int * 	(Minor Version 2)					    *
 *  Parameter4	:	unsigned short int * 	(Minor Version 3)					    *
 *  Returns	:	bool (true or false)								    *
 *  Description	:       sends the extension unit command for reading firmware version to the UVC device     *
 *			and then device sends back the firmware version will be stored in the variables	    *	
  **********************************************************************************************************
*/

bool ReadFirmwareVersion (uint8_t *pMajorVersion, uint8_t *pMinorVersion1, uint16_t *pMinorVersion2, uint16_t *pMinorVersion3)
{	

	bool timeout = true;
	int ret = 0;
	unsigned int start, end = 0;
	unsigned short int sdk_ver=0, svn_ver=0;
	
	//Initialize the buffer
	memset(g_out_packet_buf, 0x00, sizeof(g_out_packet_buf));

	//Set the Report Number
	g_out_packet_buf[1] = READFIRMWAREVERSION; 	/* Report Number */

	/* Send a Report to the Device */
	ret = write(hid_fd, g_out_packet_buf, BUFFER_LENGTH);
	if (ret < 0) {
		perror("write");
		return false;
	} else {
		printf("%s(): wrote %d bytes\n", __func__,ret);
	}
	/* Read the Firmware Version from the device */
	start = GetTickCount();
	while(timeout) 
	{	
		/* Get a report from the device */
		ret = read(hid_fd, g_in_packet_buf, BUFFER_LENGTH);
		if (ret < 0) {
			//perror("read");
		} else {
			printf("%s(): read %d bytes:\n", __func__,ret);
			if(g_in_packet_buf[0] == READFIRMWAREVERSION) {
				sdk_ver = (g_in_packet_buf[3]<<8)+g_in_packet_buf[4];
				svn_ver = (g_in_packet_buf[5]<<8)+g_in_packet_buf[6];

				*pMajorVersion = g_in_packet_buf[1];
				*pMinorVersion1 = g_in_packet_buf[2];
				*pMinorVersion2 = sdk_ver;
				*pMinorVersion3 = svn_ver;

				timeout = false;
			}
	 	}
		end = GetTickCount();
		if(end - start > TIMEOUT)
		{
			printf("%s(): Timeout occurred\n", __func__);
			timeout = false;
			return false;
		}		
	}
	return true;
}

/*
  **********************************************************************************************************
 *  MODULE TYPE	:	LIBRAY API 									    *
 *  Name	:	UninitExtensionUnit								    *
 *  Parameter1	:											    *
 *  Parameter2	:											    *
 *  Returns	:	bool (true or false)								    *
 *  Description	:       to release all the extension unit objects and other internal library objects	    *	
  **********************************************************************************************************
*/

bool UninitExtensionUnit()
{
	int ret=0;
	/* Close the hid fd */
	if(hid_fd > 0)
	{
		ret=close(hid_fd);
	}
	if(ret<0)
		return false ;
	else
		return true;
	//When threads are used don't forget to terminate them here.
}

/*
  **********************************************************************************************************
 *  MODULE TYPE	:	LIBRAY API 									    *
 *  Name	:	bus_str										    *
 *  Parameter1	:	int										    *
 *  Parameter2	:											    *
 *  Returns	:	const char *									    *
 *  Description	:       to convert integer bus type to string					 	    *	
  **********************************************************************************************************
*/
const char *bus_str(int bus)
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

/*
  **********************************************************************************************************
 *  MODULE TYPE	:	LIBRAY API 									    *
 *  Name	:	SendCameraCommand								    *
 *  Parameter1	:	unsigned int 		(Command)						    *
 *  Parameter2	:	unsigned int 		(Length)						    *
 *  Parameter3	:	unsigned int 		(Command Value)						    *
 *  Parameter4	:	unsigned int *		(pRetval)						    *
 *  Returns	:	bool (true or false)								    *
 *  Description	:       sends the extension unit command for sending a command directly to the UVC device   *
 *			and then device sends back the return value					    *	
  **********************************************************************************************************
*/
bool SendCameraCommand (uint8_t Command, uint8_t Length, uint8_t CommandValue, unsigned int *pRetval)
{
	bool timeout = true;
	int ret = 0;
	unsigned int start, end = 0;
	char CommandVal_hex[10] = "\0";

	sprintf(CommandVal_hex,"%X",CommandValue);

	//printf("\nelements are : %c %c %c %c %c %c %c\n",CommandVal_hex[0],CommandVal_hex[1],CommandVal_hex[2],CommandVal_hex[3],CommandVal_hex[4],CommandVal_hex[5],CommandVal_hex[6]);
	//printf("\nelements are : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",CommandVal_hex[0],CommandVal_hex[1],CommandVal_hex[2],CommandVal_hex[3],CommandVal_hex[4],CommandVal_hex[5],CommandVal_hex[6]);

	//Initialize the buffer
	memset(g_out_packet_buf, 0x00, sizeof(g_out_packet_buf));

	//Set the Report Number
	g_out_packet_buf[1] = FX320_CAMERA_CONTROL; 	/* Report Number */
	g_out_packet_buf[2] = FX320_CAMERA_COMMAND; 	/* Report Number */

	g_out_packet_buf[3] = (Length+1); 	/* length in hex */
	g_out_packet_buf[4] = Command;	 	/* Command in ASCII */
if (1 == Length) {
	g_out_packet_buf[5] = CommandVal_hex[0];	 	/* Hex value of CommandValue in ASCII */
} else if (2 == Length) {
	if(0x00!=CommandVal_hex[1]) {
		g_out_packet_buf[5] = CommandVal_hex[0];	 	/* Hex value of CommandValue in ASCII */
		g_out_packet_buf[6] = CommandVal_hex[1];	 	/* Hex value of CommandValue in ASCII */
	} else {
		g_out_packet_buf[5] = 0x30;		 		/* Pad extra zero in ASCII */
		g_out_packet_buf[6] = CommandVal_hex[0];	 	/* Hex value of CommandValue in ASCII */
	}
}
	/* Send a Report to the Device */
	ret = write(hid_fd, g_out_packet_buf, BUFFER_LENGTH);
	//printf("\nOut buffer : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",g_out_packet_buf[1],g_out_packet_buf[2],g_out_packet_buf[3],g_out_packet_buf[4],g_out_packet_buf[5],g_out_packet_buf[6],g_out_packet_buf[7],g_out_packet_buf[8],g_out_packet_buf[9],g_out_packet_buf[10],g_out_packet_buf[11],g_out_packet_buf[12],g_out_packet_buf[13],g_out_packet_buf[14],g_out_packet_buf[15],g_out_packet_buf[16],g_out_packet_buf[17],g_out_packet_buf[18]);
#if 1
	if (ret < 0) {
		perror("write");
		return false;
	} else {
		printf("%s(): wrote %d bytes\n", __func__,ret);
	}
	start = GetTickCount();
	while(timeout) 
	{	
		/* Get a report from the device */
		ret = read(hid_fd, g_in_packet_buf, BUFFER_LENGTH);
		if (ret < 0) {
			//perror("read");
		} else {
			printf("%s(): read %d bytes:\n", __func__,ret);
			//printf("\nIn buffer : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",g_in_packet_buf[0],g_in_packet_buf[1],g_in_packet_buf[2],g_in_packet_buf[3],g_in_packet_buf[4],g_in_packet_buf[5],g_in_packet_buf[6],g_in_packet_buf[7],g_in_packet_buf[8],g_in_packet_buf[9],g_in_packet_buf[10],g_in_packet_buf[11],g_in_packet_buf[12],g_in_packet_buf[13],g_in_packet_buf[14],g_in_packet_buf[15],g_in_packet_buf[16],g_in_packet_buf[17]);
			if(g_in_packet_buf[0] == FX320_CAMERA_CONTROL &&
				g_in_packet_buf[1] == FX320_CAMERA_COMMAND &&
				g_in_packet_buf[2] == (Length+1) &&
				g_in_packet_buf[3] == Command && 
				g_in_packet_buf[4] == g_out_packet_buf[5] &&
				g_in_packet_buf[5] == g_out_packet_buf[6] ) {

					*pRetval = g_in_packet_buf[7];

					timeout = false;
			}
		}
		end = GetTickCount();
		if(end - start > FX320_TIMEOUT)
		{
			printf("%s(): Timeout occurred\n", __func__);
			timeout = false;
			return false;
		}
	}
#endif
	return true;
}

int main(int argc, char const *argv[])
{

	bool worked = InitExtensionUnit((uint8_t*)"/dev/video1");
	unsigned int status=0;
	int ret=SendCameraCommand(SET_O_ZOOM,2,30,&status);
	if(ret == false)
	{
		printf("\nError in setting optical zoom\n");
		return false;
	} else {
		printf("Set Optical zoom to 30");
	}
	/* code */
	return 0;
}
