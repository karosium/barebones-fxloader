/*
* Barebones Intel HEX firmware loader for the Cypress FX series
*
* Author: Viktor - karosium.com <github@karosium.e4ward.com>
*
* This is free and unencumbered software released into the public domain.
*
* Anyone is free to copy, modify, publish, use, compile, sell, or
* distribute this software, either in source code form or as a compiled
* binary, for any purpose, commercial or non-commercial, and by any
* means.
*
* In jurisdictions that recognize copyright laws, the author or authors
* of this software dedicate any and all copyright interest in the
* software to the public domain. We make this dedication for the benefit
* of the public at large and to the detriment of our heirs and
* successors. We intend this dedication to be an overt act of
* relinquishment in perpetuity of all present and future rights to this
* software under copyright law.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* For more information, please refer to <http://unlicense.org/>
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libusb.h"
#include "fxloader.h"

int main(int argc, char **argv) {
	if (argc==1) {
		printf("usage: dumbloader <vid> <pid> <file.ihx>\n");
		exit(0);
	}

	libusb_device_handle *device = NULL;
	int vid, pid,filesz,s;
	FILE *firmware;	
	char *blob = malloc(65536);
	memset(blob,0,65536);

	if (libusb_init(NULL) <0) {
		printf("libusb init error\n");
		exit(1);
	}
	                 
	libusb_set_debug(NULL, 0);

	vid=strtol(argv[1],NULL,16);
	pid=strtol(argv[2],NULL,16);

	device = libusb_open_device_with_vid_pid(NULL, (uint16_t)vid, (uint16_t)pid);
	if (device==NULL) {
		printf("libusb open device error\n");
		exit(1);
	}

	libusb_set_auto_detach_kernel_driver(device, 1);
	if (libusb_claim_interface(device, 0) != LIBUSB_SUCCESS) {
		printf("libusb claim interface error\n");
		exit(1);
	}
	
  
	firmware = fopen(argv[3],"rb");
	if (firmware==NULL) {
		printf("Unable to open file\n");
		exit(1);		
	}

	fseek(firmware, 0L, SEEK_END);
	filesz = ftell(firmware);
	rewind(firmware);	

	s=fread(blob,filesz,1,firmware);

	if (s==1) {
		printf("%d bytes read from %s\n",filesz,argv[3]);
	}
 
	if (CypressUploadIhxFirmware(device,blob,filesz) == 1) { 
		printf("Firmware uploaded\n");
	} else {
		printf("Firmware upload failed\n");
	}
}
