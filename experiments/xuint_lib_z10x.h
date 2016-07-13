#ifndef XUNIT_LIB_H
#define XUNIT_LIB_H

//#include "xunit_tab_lib.h"
#include <libudev.h>
#include <stdbool.h>
#include <stdint.h>

/* Report Numbers */
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

extern int hid_fd;

unsigned char g_out_packet_buf[BUFFER_LENGTH];
unsigned char g_in_packet_buf[BUFFER_LENGTH];

const char	*hid_device;

/* Function Declarations */

bool CalculateCRC();

const char *bus_str(int);

unsigned int GetTickCount();

int find_hid_device(char *);

bool find_z10x_storage();

bool SendCameraCommand (uint8_t , uint8_t , uint8_t , uint8_t *);

#endif
