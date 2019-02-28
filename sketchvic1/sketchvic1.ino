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
SSR ssr;
unsigned long tim; //счетчик времени до перезагрузки
bool firststart = true;

void(*resetFunc) (void) = 0;

void setup() {
	
	Serial.begin(9600);//открываем ком порт для вывода отладки
	Serial.println(F("first"));

	wtr.Init(); // определяем реле подогрева воды на выход и отключаем его
	htr.Init(); // определяем реле первого контура подогрева и отключаем его
	
	lcd.Init(); // включаем подсветку дисплея и ставим курсов в начало
	#ifdef _TRACE
		Serial.println(F("Power on, arduino"));
	#endif
	vgsm3.InitGSM(lcd); //открываем порт модема  на скорости 9600
	vgsm3.InitGSM2(lcd.gsmInit);
	#ifdef _TRACE
		Serial.println(F("After InitGSM"));
	#endif
	if (!vgsm3.Reset() || !vgsm3.InitializeGprs()) resetFunc(); // вызываем команду перезагрузки устройства при первом запуске
	#ifdef _TRACE
		Serial.println(F("After reset"));
	#endif
	htr.setMaxRoomTemp(MAXROOMTEMP); // выставляем начальную температуру отключения подогрева в 25 градуса
	for (int i = 0; i < 20; i++)
	{
		htr.setRoomTemp(htr.getRoomTemp()); //заполняем температурный массив данными для точности
	}
	#ifdef _TRACE
		Serial.println(F("\nDone Init"));
	#endif
}


//resetFunc();
void loop() {
#pragma region WEEKLY REBOOT
	//добавим общую регулярную  перезагрузку  - раз в неделю
	//tim = millis();
	//if (604800000 - long(tim) <= 0)
	if (604800000 - long(millis()) <= 0)
	{
		//asm volatile ("jmp 0x0000");
		resetFunc();
	}
	Serial.println(F("Done0"));

	htr.setRoomTemp(htr.getRoomTemp()); //передаем обогревателю данные с датчика температуры в комнате
#pragma endregion
	
#pragma region CHECK MODEM ABILITY
	if (vgsm3.SendATcommand4_P(F("AT"), mdm_ok, mdm_ok, 2000) != 1) {//отправляем в модем АТ команду и читаем текущий буфер. В этот момент может прийти команда смс
		Serial.println(F("No AT. Reset"));
		ssr.ErrorBlink();
		resetFunc(); //пока модем не откликнется будем перезапускать ардуино
	}
	Serial.println(F("MODEM OK"));
	lcd.Status(0, htr.getTempArr(), true, wtr.getStarted(), true, htr.getStarted(), true);
#pragma endregion
#pragma region FIRST START

	if (firststart) {// на первом запуске шлем SMS команду пользователю, что живы
		//vgsm2.SendInitSMS(vgsm2.phone, vgsm2.phone2);
		vgsm3.SendInitSMSChr();
		firststart = false; // чтобы не слать повторно при следующем цикле
	}
#pragma endregion
#pragma region SMS CHECK
	vgsm3.SMSCheckNewMsg();
	if (vgsm3.sms_index > 0) {// при очередном опросе SendATCommand получили информацию, что пришло сообщение
//тест отправки, что нашли входящий смс, не важно какой
		vgsm3.SendIndexSMSChr();// отправляем проверочный номер смс в памяти телефона
		vgsm3.SMSRead();//читаем данное сообщение из модема в строку
		Serial.println(F("Done1"));

		if (vgsm3.CheckSMSCommand(htr, wtr)) { // проверяем приход sms комманды, в случае прихода выставляем флаги для дальнейшей обработки действий
			Serial.println(F("Done3"));

			//wtr.checkWater();// отрабатываем флаг команды включения подогрева воды
			htr.checkHeat();// отрабатываем флаг включения обогревателя
			vgsm3.StatusChr(htr.getTempArr(), wtr.getStarted(), htr.getStarted(1), ssr.freeRam()); // если пришла удаленная команда, формируем ответ
		}
	}
#pragma endregion
#pragma region REST
	//отправляем данные на http сервер
	if (vgsm3.SendATcommand4_P(F("AT+CIFSR"), mdm_ip_ok, mdm_error, 10000) == 1) //отправляем в модем команду на получение IP адреса
	{	
		Serial.println("IP OK");
	}
	else//иначе  SIM900 был выключен или чтото пошло не так
	{  //Если SIM900 не отзывается на самую простую команду он точно выключен или висит
		if (vgsm3.SendATcommand4_P(F("AT"), mdm_ok, mdm_ok, 2000) != 1)
		{
			Serial.println(F("SIM900 OFF...")); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
			//power_on();
			resetFunc();// полностью перегружаем arduino
		}
		Serial.println(F("Connecting to the GSM network...")); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		while (vgsm3.SendATcommand4_P(F("AT+CREG?"), mdm_ntw_reg_1, mdm_ntw_reg_5, 1000) == 0); //пока не получим статус нормальной регистрации в сети проводим инициализацию
		//настраиваем sim900
		vgsm3.InitializeGprs();  //организуем UDP соединение с narodmon.ru:8283
	}
	if (vgsm3.SendATcommand4_P(F("AT"), mdm_ok, mdm_ok, 2000) == 1)
	{
		////Передаём данные на сервер

		vgsm3.TCPSendData2();//отправляем данные на сервер  и получаем ответ
		////Продолжаем измерение
		////sensor_filtr_read(PERIOD, DEPTH_FILTRE);
	}
	if (vgsm3.tcp_index > 0) {// получили информацию от сервера по включению или отключению отопления
		vgsm3.CheckTCPCommand(htr, wtr);//выставляем флаги
	}
#pragma endregion

	htr.checkHeat(); //проверяем только обогрев, так как он может быть уже включен
	
	ssr.Blink();
	Serial.print("free memory - ");
	Serial.println(ssr.freeRam());

	delay(5000); //ждем 5 секунд и делаем новый опрос -поменять на 30
}



