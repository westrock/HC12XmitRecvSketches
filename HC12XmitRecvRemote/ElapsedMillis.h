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
	ElapsedMillisClass();
	ElapsedMillisClass(unsigned long newStart);
	void start();
	void setStart(unsigned long newStart);
	unsigned long getStart();
	unsigned long elapsedMillis();
	unsigned long elapsedMillis(unsigned long now);
	unsigned long elapsedSeconds();
	unsigned long elapsedSeconds(unsigned long now);

	static unsigned long elapsedMillis(unsigned long likelyCurrentTime, unsigned long likelyStartTime);
	static unsigned long elapsedSeconds(unsigned long likelyCurrentTime, unsigned long likelyStartTime);
};

extern ElapsedMillisClass ElapsedMillis;

#endif

