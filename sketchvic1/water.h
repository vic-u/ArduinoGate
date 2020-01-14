#include "def.h"
#include <arduino.h>
#ifndef VIC_WTR
#define VIC_WTR



// РєР»Р°СЃСЃ СѓРїСЂР°РІР»РµРЅРёСЏ РїРѕРґРѕРіСЂРµРІРѕРј РІРѕРґС‹
class Water {
private:
  boolean water_started = false; // С„Р»Р°Рі СѓРєР°Р·С‹РІР°РµС‚ С‚РµРєСѓС‰РµРµ СЃРѕСЃС‚РѕСЏРЅРёРµ СЂРµР»Рµ, РІРєР»СЋС‡РµРЅРѕ РёР»Рё РЅРµС‚
  int water_command = RC_NOTHING; // РєРѕРјР°РЅРґР° СѓРїР°СЂРІР»РµРЅРёСЏ СЂРµР»Рµ. РёР·РЅР°С‡Р°Р»СЊРЅРѕ РЅРµРѕРїСЂРµРґРµР»РµРЅР°. РјРѕР¶РµС‚ Р±С‹С‚СЊ РєРѕРјР°РЅРґР° РЅР° РІРєР»СЋС‡РµРЅРёРµ РёР»Рё РѕС‚РєР»СЋС‡РµРЅРёРµ СЂРµР»Рµ
public:
  Water(){};
  ~Water(){};
  void checkWater();
  //IDevice
  virtual void setCommand(int command);
  virtual void setMaxTemp(int temp) {};
  virtual void setDeltaTemp(int delta) {};
  virtual int getMaxTemp() { return 0; };
  virtual int getDeltaTemp() { return 0; };

  boolean getStarted();
  void Init();
};
#endif
