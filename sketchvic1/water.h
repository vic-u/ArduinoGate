#include "def.h"
#include <arduino.h>
#ifndef VIC_WTR
#define VIC_WTR



// класс управления подогревом воды
class Water {
private:
  boolean water_started = false; // флаг указывает текущее состояние реле,
  int water_command = RC_NOTHING; // команда упарвления реле. изначально неопределена. может быть команда на включение или отключение реле
public:
  Water(){};
  ~Water(){};
  void checkWater();
  virtual void setCommand(int command);
  virtual void setMaxTemp(int temp) {};
  virtual void setDeltaTemp(int delta) {};
  virtual int getMaxTemp() { return 0; };
  virtual int getDeltaTemp() { return 0; };
  boolean getStarted();
  void Init();
};
#endif
