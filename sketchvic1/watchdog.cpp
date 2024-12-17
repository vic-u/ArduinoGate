#include "watchdog.h"

WatchDog::WatchDog(uint8_t signal_pin)
{
	pin = signal_pin;
}

void WatchDog::init()
{
	pinMode(pin, OUTPUT);                   // Конфигурируем вывод подключённый к сторожевому таймеру как выход
	digitalWrite(pin, LOW);
}

void WatchDog::reset()
{
	digitalWrite(pin, HIGH);                 // Формируем импульс на входе сторожевого таймера
	digitalWrite(pin, LOW);
}
