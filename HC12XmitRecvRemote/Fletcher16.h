// Fletcher16.h

#ifndef _FLETCHER16_h
#define _FLETCHER16_h

#include <stdint.h>

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

uint16_t Fletcher16(const uint8_t* data, size_t len);
uint16_t Fletcher16AddCB(uint8_t* data, size_t len);
uint16_t GetFletcher16CheckBytes(const uint8_t* data, size_t len);

union FletcherCheckBytes {
	uint16_t bothBytes16;
	uint8_t  checkbyte8[2];
};


#endif

