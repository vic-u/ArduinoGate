/* 
v8 базовая функциональность
v9 
	добавлена проверка,что входящее сообщения приходят только с нужного номера
	добавлено логирование на LCD дисплей в четвертую строку
v10 
	переделана отправка Send SMSChr
	заменено все на printf_P
	исправлена ошибка SMSCheckNewMsg при получениие больших смс на русском
v11
	вернулось время перезагрузки на неделю
	переделан лог на LCD
	снова включено гпрс 
v12 
	увеличен буфер для Software Serial до 128 Кб
	выводится на дисплей дополнительная информация о макс темпе и дельта темпе
	исправлена ошибка для дельта темпа
*/
//#include <DHT.h>

#include <Wire.h>
#include <SoftwareSerial.h>

#include "def.h"
#include "water.h"
#include "heat.h"
#include "mylcd.h"
#include "VGSM3.h"
/////////////////////////////////////////////////
Water wtr; // объект класса управления водонагревателем
Heater htr; // объект класс управления нагревателем
MYLCD lcd(0x3F, 20, 4); //объект экран в четыре строчки
VGSM3 vgsm3;// модуль гсм 800 л
SSR ssr; //управление реле
bool firststart = true; //флаг первого прохода цикла, чтобы отправить
bool gprsactive = false;//флаг включение конала интернета в дополнение к смс

void(*resetFunc) (void) = 0; //перезагрузочная функция


void setup() {
	
	Serial.begin(9600);//открываем ком порт для вывода отладки
#ifdef _TRACE	
	Serial.println(F("first"));
	lcd.log(F("first"));
#endif
	wtr.Init(); // определяем реле подогрева воды на выход и отключаем его
	htr.Init(); // определяем реле первого контура подогрева и отключаем его	
	lcd.Init(); // включаем подсветку дисплея и ставим курсор в начало
	#ifdef _TRACE
		Serial.println(F("Power on, arduino"));
		lcd.log(F("Power on, arduino"));
	#endif
	vgsm3.Init(lcd); //открываем порт модема  на скорости 9600 и выводим на дисплей
	#ifdef _TRACE
		Serial.println(F("After InitGSM"));
		lcd.log(F("After InitGSM"));
	#endif
	if (!vgsm3.InitGSM()) resetFunc(); // проверяем инициализацию ГСМ и ГПРС, не смогли перегружаем устройство и модем
	#ifdef _TRACE
		Serial.println(F("After reset"));
		lcd.log(F("After reset"));
	#endif
	// модем нам нужен обязательно а gprs доп функция
	gprsactive = vgsm3.InitGPRS(); //дополнительно включаем инициализацию для интернета
	htr.max_room_temp = MAXROOMTEMP; // выставляем начальную температуру отключения
	for (int i = 0; i < 20; i++)
	{
		htr.setRoomTemp(htr.getRoomTemp()); //заполняем температурный массив данными
	}
	#ifdef _TRACE
		Serial.println(F("Done Init"));
		lcd.log(F("Done Init"));
	#endif
}

void loop() {
#pragma region WEEKLY REBOOT
	//добавим общую регулярную  перезагрузку  - раз в неделю
	//if (259200000 - long(millis()) <= 0)
	//if ((604800000*2) - long(millis()) <= 0) {
		if ((604800000) - long(millis()) <= 0) {
		resetFunc();
	}
	htr.setRoomTemp(htr.getRoomTemp()); //передаем обогревателю данные с датчика
#pragma endregion
#pragma region CHECK MODEM ABILITY
	if (vgsm3.SendATcommand4(F("AT"), mdm_ok, mdm_ok, WT4) != 1) {//отправляем в модем АТ команду и читаем текущий буфер. В этот момент может прийти команда смс
#ifdef _TRACE
		Serial.println(F("No AT. Reset"));
		lcd.log(F("No AT. Reset"));
#endif
		ssr.ErrorBlink();
		resetFunc(); //пока модем не откликнется будем перезапускать ардуино
	}
#ifdef _TRACE
	Serial.println(F("MODEM OK"));
	lcd.log(F("MODEM OK"));
#endif
	lcd.Status(htr.getTempArr(), wtr.getStarted(), htr.heat_started, gprsactive, htr.max_room_temp, htr.delta_temp); // на LCD экран выводим информацию
#pragma endregion
#pragma region FIRST START

	if (firststart) {// на первом запуске шлем SMS команду пользователю, что живы
		vgsm3.SendInitSMSChr();
		firststart = false; // чтобы не слать повторно при следующем цикле
	}
#pragma endregion
	if (!gprsactive) {
		lcd.log(F("InitGPRS"));
		gprsactive =  vgsm3.InitGPRS(); //если в предыдущий заход отправка по tcp не удалась, попробуем запустить gprs
	}
#pragma region SMS CHECK
	if (vgsm3.SMSCheckNewMsg()) {// проверяем наличие UNREAD сообщений;
#ifdef _TRACE
		Serial.println(F("NewSMS")); //Done1
		lcd.log(F("NewSMS"));
#endif
		if (vgsm3.CheckSMSCommand(htr, wtr, vgsm3.chf)) { // проверяем приход sms комманды, в случае прихода выставляем флаги для дальнейшей обработки действий
#ifdef _TRACE
			Serial.println(F("NewCom")); //Done3
			lcd.log(F("NewCom"));
#endif
			htr.checkHeat();// отрабатываем флаг включения обогревателя
			vgsm3.StatusChr(htr.getTempArr(), wtr.getStarted(), htr.getStarted(1), ssr.freeRam()); // если пришла удаленная команда, формируем ответ
		}
	}
#pragma endregion
#pragma region TCP SEND
	if (gprsactive) gprsactive = vgsm3.TCPSendData2(htr.getTempArr(), htr.getStarted(1), htr, vgsm3.chf);
#pragma endregion
	htr.checkHeat(); //проверяем только обогрев, так как он может быть уже включен
	
	ssr.Blink();
#ifdef _TRACE
	Serial.print(F("free memory - "));
	Serial.println(ssr.freeRam());
	lcd.log(ssr.freeRam());
#endif
	delay(30000); //ждем 5 секунд и делаем новый опрос -поменять на 30
}
