// OledHelper.h

#pragma	once
#ifndef _OLEDHELPER_h
#define _OLEDHELPER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class OledHelperClass
{
 protected:
	 Adafruit_SSD1306* _display = NULL;
	 void printlnCharsCommon(bool isPrintLn, const char* buffer, bool clear = false, int16_t col = -1, int16_t row = -1);

 public:
	 void init(Adafruit_SSD1306* withDisplay);
	 void startup(bool showSplash = false);
	 void setCursor(int16_t col = 0, int16_t row = 0);
	 void splash(char* buffer);
	 void printlnChars(const char* buffer, bool clear = false, int16_t col = -1, int16_t row = -1);
	 void printChars(const char* buffer, bool clear = false, int16_t col = -1, int16_t row = -1);
	 void clear();
};

//extern OledHelperClass OledHelper;

#endif

