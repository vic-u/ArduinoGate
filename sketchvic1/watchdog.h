#include <arduino.h>
#pragma once
class WatchDog
{
private:
	uint8_t pin;
public:
	WatchDog(uint8_t signal_pin);
	void init();
	void reset();
};

