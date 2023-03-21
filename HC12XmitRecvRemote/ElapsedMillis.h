// ElapsedMillis.h

#pragma once
#ifndef _ELAPSEDMILLIS_h
#define _ELAPSEDMILLIS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <limits.h>

class ElapsedMillisClass
{
protected:
	 unsigned long _startTime;

public:
	void start();
	void ElapsedMillisClass::setStart(unsigned long newStart);
	unsigned long ElapsedMillisClass::getStart();
	unsigned long ElapsedMillisClass::elapsedMillis();
	unsigned long ElapsedMillisClass::elapsedMillis(unsigned long now);
	unsigned long ElapsedMillisClass::elapsedSeconds();
	unsigned long ElapsedMillisClass::elapsedSeconds(unsigned long now);

	static unsigned long ElapsedMillisClass::elapsedMillis(unsigned long fromTime, unsigned long toTime);
	static unsigned long ElapsedMillisClass::elapsedSeconds(unsigned long fromTime, unsigned long toTime);
};

extern ElapsedMillisClass ElapsedMillis;

#endif

