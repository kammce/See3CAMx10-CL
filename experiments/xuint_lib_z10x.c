#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>      
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/input.h>
#include <linux/hidraw.h>

#include "xuint_lib_z10x.h"

/*
  **********************************************************************************************************
 *  MODULE TYPE	:	LIBRAY API 									    *
 *  Name	:	ReadCameraFirmwareVersion							    *
 *  Parameter1	:	unsigned char *		(FirmwareVersion)					    *
 *  Returns	:	bool (true or false)								    *
 *  Description	:       sends the extension unit command for reading firmware version to the UVC device     *
 *			and then device sends back the firmware version which is stored in the variable     *	
  **********************************************************************************************************
*/

bool ReadCameraFirmwareVersion (char *FirmwareVersion)
{
	bool timeout = true;
	int ret = 0;
	unsigned int start, end = 0;
	FirmwareVersion[BUFFER_LENGTH]='\0';

	//Initialize the buffer
	memset(g_out_packet_buf, 0x00, sizeof(g_out_packet_buf));
	memset(FirmwareVersion, 0x00, sizeof(FirmwareVersion[BUFFER_LENGTH]));

	//Set the Report Number
	g_out_packet_buf[1] = FX320_CAMERA_CONTROL; 	/* Report Number */
	g_out_packet_buf[2] = FX320_FIRMWARE_VERSION; 	/* Report Number */

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
			if(g_in_packet_buf[0] == FX320_CAMERA_CONTROL &&
				g_in_packet_buf[1] == FX320_FIRMWARE_VERSION ) {
			//printf("\nIn buffer : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",g_in_packet_buf[0],g_in_packet_buf[1],g_in_packet_buf[2],g_in_packet_buf[3],g_in_packet_buf[4],g_in_packet_buf[5],g_in_packet_buf[6],g_in_packet_buf[7],g_in_packet_buf[8],g_in_packet_buf[9],g_in_packet_buf[10],g_in_packet_buf[11],g_in_packet_buf[12],g_in_packet_buf[13],g_in_packet_buf[14],g_in_packet_buf[15],g_in_packet_buf[16],g_in_packet_buf[17]);
					int i=0;
					while(CARRIAGE_RETURN != g_in_packet_buf[i+2] && i<10) {
						FirmwareVersion[i]=g_in_packet_buf[i+2];
						printf("\n0x%x\t%c",FirmwareVersion[i], FirmwareVersion[i]);
						i++;
					}
					FirmwareVersion[i]=0x00;
					printf("\n0x%x\t%c",FirmwareVersion[i], FirmwareVersion[i]);
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
	return true;

}

/*
  **********************************************************************************************************
 *  MODULE TYPE	:	LIBRAY API 									    *
 *  Name	:	RestoreFactorySettings								    *
 *  Parameter1	:	unsigned int *		(pRetval)						    *
 *  Returns	:	bool (true or false)								    *
 *  Description	:       sends the extension unit command for restoring all controls to their default	    *
 *			values to the UVC device and then the device sends back return value		    *	
  **********************************************************************************************************
*/

bool RestoreFactorySettings (uint8_t *pRetval)
{
	bool timeout = true;
	int ret = 0;
	unsigned int start, end = 0;

	//Initialize the buffer
	memset(g_out_packet_buf, 0x00, sizeof(g_out_packet_buf));

	//Set the Report Number
	g_out_packet_buf[1] = FX320_CAMERA_CONTROL; 	/* Report Number */
	g_out_packet_buf[2] = FX320_CAMERA_COMMAND; 	/* Report Number */
	g_out_packet_buf[3] = 0x01; 			/* Length */
	g_out_packet_buf[4] = FX320_RESET; 		/* Report Number */

	/* Send a Report to the Device */
	ret = write(hid_fd, g_out_packet_buf, BUFFER_LENGTH);
	//printf("\nOut buffer : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",g_out_packet_buf[1],g_out_packet_buf[2],g_out_packet_buf[3],g_out_packet_buf[4],g_out_packet_buf[5],g_out_packet_buf[6],g_out_packet_buf[7],g_out_packet_buf[8],g_out_packet_buf[9],g_out_packet_buf[10],g_out_packet_buf[11],g_out_packet_buf[12],g_out_packet_buf[13],g_out_packet_buf[14],g_out_packet_buf[15],g_out_packet_buf[16],g_out_packet_buf[17],g_out_packet_buf[18]);
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
				g_in_packet_buf[2] == 0x01 &&
				g_in_packet_buf[3] == FX320_RESET ) {
					*pRetval = g_in_packet_buf[7];
					timeout = false;
			}
		}
		end = GetTickCount();
		if(end - start > FX320_RESET_TIMEOUT)
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
bool SendCameraCommand (uint8_t Command, uint8_t Length, uint8_t CommandValue, uint8_t *pRetval)
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

/*
  **********************************************************************************************************
 *  MODULE TYPE	:	INTERNAL API 									    *
 *  Name	:	find_z10x_storage								    *
 *  Parameter1	:	char * 	(Storage Name)								    *
 *  Returns	:	bool (true or false)								    *
 *  Description	:       To find the mass storage device that is enumerated when image capture is done using *
 *			See3CAM_30Z10X USB 3.0 camera							    *
  **********************************************************************************************************
*/

bool find_z10x_storage(char *mass_storage)
{
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev, *pdev;
	int ret = 0;
	const char *stor;
		const char *path;

  	/* Create the udev object */
	udev = udev_new();
	if (!udev) {
		printf("Can't create udev\n");
		exit(1);
	}

	enumerate = udev_enumerate_new(udev);
	ret = udev_enumerate_add_match_subsystem(enumerate, "block");
	if(ret < 0)
		return false;
	udev_enumerate_scan_devices(enumerate);
	if(ret < 0)
		return false;
	devices = udev_enumerate_get_list_entry(enumerate);
	if(!devices)
		return false;
	udev_list_entry_foreach(dev_list_entry, devices) {
		if(ret < 0)
			return false;
	
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);
		pdev = udev_device_get_parent_with_subsystem_devtype(
		       dev,
		       "usb",
		       "usb_device");
		if (!pdev) {
			printf("Unable to find parent usb device.");
			return false;
		}
		if(!strncmp(udev_device_get_sysattr_value(pdev,"idVendor"), "2560", 4) && !strncmp(udev_device_get_sysattr_value(pdev, "idProduct"), "a270", 4))
		{
			stor = udev_device_get_devnode(dev);
			udev_device_unref(pdev);
			strcpy(mass_storage,stor); 
			strncat(mass_storage,"1",1);
			printf("\nmass storage node :%s\n",mass_storage);
			stor = NULL;
			path = NULL;
			udev_enumerate_unref(enumerate);
			udev_unref(udev);
			return true;
		}
	}
	stor = NULL;
	path = NULL;
	udev_device_unref(pdev);
	udev_enumerate_unref(enumerate);
	udev_unref(udev);
	return false;
}
