#include "water.h"

void Water::setCommand(int command) 
{
  water_command = command;
}
void Water::checkWater() // включение и отключение  подогрева воды
{
	if (water_started) 
	{
		digitalWrite(SSR_W, LOW);
	}
	else
	{
		digitalWrite(SSR_W, HIGH);
	}
  if(water_command == RC_DEVICEON) 
  {
    digitalWrite(SSR_W, LOW);
    water_started = true;
    water_command = RC_NOTHING;
    #ifdef _TRACE 
      Serial.println("water boiler start"); 
    #endif
    return;
  }
  if(water_command == RC_DEVICEOFF)
  {
    digitalWrite(SSR_W, HIGH);
    water_started = false;
    water_command = RC_NOTHING;
    #ifdef _TRACE 
      Serial.println(F("water boiler stop")); 
    #endif
    return;    
  }
  delay(500);
}
boolean Water::getStarted()
{
  return water_started;
}

void Water::Init()
{
  pinMode(SSR_W, OUTPUT);
  delay(500);
  digitalWrite(SSR_W, HIGH);
  #ifdef _TRACE
  Serial.println(F("water init")); 
  #endif
}
