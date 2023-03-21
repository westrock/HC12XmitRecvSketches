// 
// 
// 

#include "ElapsedMillis.h"

void ElapsedMillisClass::start()
{
	_startTime = millis();
}


void ElapsedMillisClass::setStart(unsigned long newStart)
{
	_startTime = newStart;
}


unsigned long ElapsedMillisClass::getStart()
{
	return _startTime;
}


unsigned long ElapsedMillisClass::elapsedMillis() {
	return elapsedMillis(millis(), _startTime);
}


unsigned long ElapsedMillisClass::elapsedMillis(unsigned long now) {
	return elapsedMillis(now, _startTime);
}


unsigned long ElapsedMillisClass::elapsedMillis(unsigned long fromTime, unsigned long toTime) {
	if (toTime >= fromTime) {
		return toTime - fromTime;
	}
	else {
		return toTime + (ULONG_MAX - fromTime);
	}
}


unsigned long ElapsedMillisClass::elapsedSeconds() {
	return elapsedSeconds(millis(), _startTime);
}


unsigned long ElapsedMillisClass::elapsedSeconds(unsigned long now) {
	return elapsedSeconds(now, _startTime);
}


unsigned long ElapsedMillisClass::elapsedSeconds(unsigned long fromTime, unsigned long toTime) {
	return elapsedMillis(fromTime, toTime) / 1000L;
}


ElapsedMillisClass ElapsedMillis;

