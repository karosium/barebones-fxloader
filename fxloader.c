/*
* Barebones Intel HEX firmware loader for the Cypress FX2 series
*
* Author: Viktor - karosium.com <github@karosium.e4ward.com>
*
* public domain parse_hex_line() courtesy of Paul Stoffregen, paul@ece.orst.edu
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

static unsigned int reset_address = 0xE600;

int parse_hex_line(char *theline, char bytes[], int *addr, int *num,  int *code)
{
	int sum, len, cksum;
	char *ptr;

	*num = 0;
	if (theline[0] != ':') return 0;
	if (strlen(theline) < 11) return 0;

	ptr = theline+1;
	if (!sscanf(ptr, "%02x", &len)) return 0;
	ptr += 2;
	if ( strlen(theline) < (11 + (len * 2)) ) return 0;
	if (!sscanf(ptr, "%04x", addr)) return 0;
	ptr += 4;
	//  printf("Line: length=%d Addr=%d\n", len, *addr); 
	if (!sscanf(ptr, "%02x", code)) return 0;
	ptr += 2;
	sum = (len & 255) + ((*addr >> 8) & 255) + (*addr & 255) + (*code & 255);
	while(*num != len) {
		if (!sscanf(ptr, "%02x", (int *)&bytes[*num])) return 0;
		ptr += 2;
		sum += bytes[*num] & 255;
		(*num)++;
		if (*num >= 256) return 0;
	}
	if (!sscanf(ptr, "%02x", &cksum)) return 0;
	if ( ((sum & 255) + (cksum & 255)) & 255 ) return 0; /* checksum error */
	return 1;
}

void CypressSetResetAddress(unsigned int address) {
	reset_address = address;
}
int CypressWriteRam(libusb_device_handle *device,unsigned int addr, unsigned char *buf, unsigned int len) {
	int status;

	status = libusb_control_transfer(device,
					LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
					0xA0,
					addr&0xFFFF, 
					addr>>16,
					buf, 
					len, 
					1000);
	return status;

}

int CypressReset(libusb_device_handle *device,unsigned char suspended) {
	return CypressWriteRam(device,reset_address,&suspended,1);
}

int CypressUploadIhxFirmware(libusb_device_handle *device, char *buf, unsigned int len) {
	char bytes[256];
	char *tempBuf;

	char *line="";
	int i,addr,num,code;

	tempBuf = malloc(len+1);
	tempBuf[len] = 0;
	memcpy(tempBuf,buf,len);

	line = strtok(tempBuf,"\n");	

	i=CypressReset(device,1);

	if (i<0) {
		free(tempBuf);
	 	return i;
	}
	while ((line != NULL) && (code !=1)) {
		parse_hex_line(line,bytes,&addr,&num,&code);

		if (code==0) {
			i=CypressWriteRam(device,addr,(unsigned char*)&bytes,num);
			if (i<0) {
				free(tempBuf);
		 		return i;
			}

		}
		if (code !=1)
			line = strtok(NULL,"\n");		
	}
	free(tempBuf);
	i=CypressReset(device,0);
	if (i<0) {
		 return i;
	}
	return 1;
}
