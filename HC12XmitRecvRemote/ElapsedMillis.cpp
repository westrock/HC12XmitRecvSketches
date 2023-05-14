// 
// 
// 

#include "ElapsedMillis.h"

ElapsedMillisClass::ElapsedMillisClass()
{
	_startTime = millis();
}


ElapsedMillisClass::ElapsedMillisClass(unsigned long newStart)
{
	_startTime = newStart;
}


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


unsigned long ElapsedMillisClass::elapsedMillis(unsigned long likelyCurrentTime, unsigned long likelyStartTime) {
	unsigned long deltaMillis;

	if (likelyCurrentTime >= likelyStartTime) {
		deltaMillis = likelyCurrentTime - likelyStartTime;
	}
	else {
		deltaMillis = likelyCurrentTime + (ULONG_MAX - likelyStartTime);
	}
	return deltaMillis;
}


unsigned long ElapsedMillisClass::elapsedSeconds() {
	return elapsedSeconds(millis(), _startTime);
}


unsigned long ElapsedMillisClass::elapsedSeconds(unsigned long now) {
	return elapsedSeconds(now, _startTime);
}


unsigned long ElapsedMillisClass::elapsedSeconds(unsigned long likelyCurrentTime, unsigned long likelyStartTime) {
	return elapsedMillis(likelyCurrentTime, likelyStartTime) / 1000L;
}


ElapsedMillisClass ElapsedMillis;

