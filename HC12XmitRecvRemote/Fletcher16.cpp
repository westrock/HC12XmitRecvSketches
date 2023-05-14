// 
// 
// 

#include "Fletcher16.h"
#include <stdint.h>


uint16_t Fletcher16(const uint8_t* data, size_t len) {
	uint32_t c0, c1;

	/*  Found by solving for c1 overflow: */
	/* n > 0 and n * (n+1) / 2 * (2^8-1) < (2^32-1). */
	for (c0 = c1 = 0; len > 0; ) {
		size_t blocklen = len;
		if (blocklen > 5802) {
			blocklen = 5802;
		}
		len -= blocklen;
		do {
			c0 = c0 + *data++;
			c1 = c1 + c0;
		} while (--blocklen);
		c0 = c0 % 255;
		c1 = c1 % 255;
	}
	return (c1 << 8 | c0);
}


uint16_t Fletcher16AddCB(uint8_t* data, size_t len) {
	uint16_t csum, checkBytes;
	uint16_t f0, f1;
	uint8_t cb0, cb1;

	csum = Fletcher16((const uint8_t*)data, len);
	f0 = csum & 0xff;
	f1 = (csum >> 8) & 0xff;
	cb0 = 0xff - ((f0 + f1) % 0xff);
	cb1 = 0xff - ((f0 + cb0) % 0xff);

	data[len++] = cb0;
	data[len++] = cb1;
	data[len] = 0;

	return csum;
}


uint16_t Fletcher16AddCB_old(uint8_t* data, size_t len) {
	uint16_t checkBytes;
	uint8_t cb0, cb1;

	checkBytes = GetFletcher16CheckBytes((const uint8_t*)data, len);
	cb0 = checkBytes & 0xff;
	cb1 = (checkBytes >> 8) & 0xff;

	data[len++] = cb0;
	data[len++] = cb1;
	data[len] = 0;
	return checkBytes;
}


uint16_t GetFletcher16CheckBytes(const uint8_t* data, size_t len) {
	uint16_t csum, checkBytes;
	uint16_t f0, f1;
	uint8_t c0, c1;

	csum = Fletcher16((const uint8_t*)data, len);
	f0 = csum & 0xff;
	f1 = (csum >> 8) & 0xff;
	c0 = 0xff - ((f0 + f1) % 0xff);
	c1 = 0xff - ((f0 + c0) % 0xff);

	checkBytes = (c1 << 8) | c0;
	return checkBytes;
}
