/* ВНИМАНИЕ  Для перепрограммирования требуется отключение arduino от шилда.
* доработано для отправки данных датчиков на сайт narodmon.ru
* Используемые датчики
* DHT - 22
* SIM900 arduino шидлов мне известно 2 типа
* у моего сигнал ON - OFF на пине D9
* у другого сигнал ON - OFF на пине D8
* следует поправить int onSIM900Pin = 9 или 8;
* В данной реализации использован апаратный RT - TX порт поэтому
* Перемычки RT - TX на плате следует установить в положение HW(HardWare)
* ******************************************************************************************
____________________________________________________
здесь следует изменить MAC_ADDRESS и APN по своему разумению
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
В соответствии с протоколом <a href = "http://narodmon.ru/#proto" title = "http://narodmon.ru/#proto" rel = "nofollow">http://narodmon.ru/#proto</a>
например 11 - 22 - 33 - AA - CC - FF  112233AACCFF  11 : 22 : 33 : AA : CC : FF с точки зрения сервера тождественны.
тире и двоеточия не обязательны, поскольку проигнорируются сервером - проверено.
******************************************************************************************* /
// Mac устройства можно генерировать вот тут <a href="http://www.miniwebtool.com/mac-address-generator/" title="http://www.miniwebtool.com/mac-address-generator/" rel="nofollow">http://www.miniwebtool.com/mac-address-generator/</a>
#define  MAC_ADDRESS "112233AACCFF" //этот незабудь поправить - поскольку уже занят
/*
Настроим модем SIM900 под своего оператора связи
<a href="http://www.gpshome.ru/gprs_apn" title="http://www.gpshome.ru/gprs_apn" rel="nofollow">http://www.gpshome.ru/gprs_apn</a>
*/
//команда - конфигурации модема под оператора MTS
#define command_APN "AT+CSTT=\"internet.mts.ru\",\"mts\",\"mts\""
//команда - конфигурации модема под оператора beeline
//#define command_APN "AT+CSTT=\"\",\"beeline\",\"beeline\""
//команда - конфигурации модема под оператора МегаФон
//#define command_APN "AT+CSTT=\"internet\",\"gdata\",\"gdata\""
//команда - конфигурации модема под оператора Теле2
//#define command_APN "AT+CSTT=\"internet.tele2.ru\",\"\",\"\""
#define DEPTH_FILTRE 15//глубина фильтрации
#define PERIOD 58 //период опроса датчика 58 секунд.
// период отправки сообщений в 58 секунд умноженный на 15 (глубина фильтрации)итого ~ 15 минут +-чуток
/*
Скорость Апаратного последовательного  порта
Иногда SIM900 приходит без включенной опции автоопределения скорости порта
Следует включить автоопределение скорости
Либо назначить принудительно.
Это делается так -
AT+IPR? говорит какая скорость
AT+IPR=0              авто
AT+IPR=19200          19200
Если это не под силу , то можно влоб - Брутфорсом.
То-есть подобрать скорость экспериментально из ряда -
1200, 2400, 4800, 9600, 19200, 38400, 57600 , 115200
На этой-же скорости во время работы устройства
в мониторе терминала будет выводиться лог хода работы.
Factory baudrate setting is auto-bauding by default.
Baudrate can be fixed using the command AT+IPR=baudrate.
Allowed baudrates:
0 (Auto-bauding) , 1200 , 2400 , 4800 , 9600 , 19200 , 38400 , 57600 and 115200 ;  */
#define SPEED_RXTX 19200
/*
* команда - что, куда, и в какой порт слать будем
* Мне всегда нравился протокол UDP . Выстрелил - забыл.
* Пропадание десятка сообщений в сутки из сотни - погоды не сделает :) */
#define data_ip_protocol "AT+CIPSTART=\"UDP\",\"narodmon.ru\",\"8283\""
//Идентификаторы датчиков
#define  SOURCE_1 "#T1#" // Температура DHT
#define  SOURCE_2 "#H1#" // Влажность DHT
#define  SOURCE_3 "#T2#" // Температура BMP085
#define  SOURCE_4 "#P1#" // Давление BMP085
#define  SOURCE_5 "#Z1#" // Расчётная точка росы
#define  SOURCE_6 "#Z2#" // время работы прибора в часах
#define endCODE "##"
unsigned long tim;
//Подключаем требуемые библиотеки
#include <Wire.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;
// Connect VCC of the BMP085 sensor to 3.3V (NOT 5.0V!)
// Connect GND to Ground
// Connect SCL to i2c clock - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 5
// Connect SDA to i2c data - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 4
// EOC is not used, it signifies an end of conversion
// XCLR is a reset pin, also not used here
// Получение температуры с BMP085  bmp.readTemperature();
// Получение давления с BMP085 в паскалях  bmp.readPressure();
float tempeBMP085_f = 0, tempeBMP085TMP = 0, pressBMP085_f = 0, pressBMP085TMP = 0, timems;
#include <DHT.h>
#define PIN_DHT 2   
// what pin we're connected to
// Uncomment whatever type you're using!
//#define TYPE_DHT DHT11   // DHT 11
#define TYPE_DHT DHT22   // DHT 22  (AM2302)
//#define TYPE_DHT DHT21   // DHT 21 (AM2301)
// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your PIN_DHT is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor
// У меня DHT22 на столе , на 5 метровом кабеле работало и без резистора
float temperature_f = 0, humidity_f = 0, temperatureTMP = 0, humidity_fTMP = 0;
DHT dht(PIN_DHT, TYPE_DHT);
int8_t answer;
//PIN включения GSM модуля после пропадания питания
int onSIM900Pin = 9; //у моего модуля сигнал ON-OFF SIM900 на пине D9 ARDUINO
//Выделение буфера для строковых переменных
char aux_str[20];
char buf_ip_data[150]; //Пока на 2 датчика хватит({Хотя сообщение не больше 70 байт)
int ledPin = 13; //сигнальное устройство - светодиод
/*===========================================================================================================*/
void setup()
{

		pinMode(ledPin, OUTPUT);
		pinMode(onSIM900Pin, OUTPUT);
		// pinMode(resetSIM900Pin, OUTPUT);
		Serial.begin(SPEED_RXTX); // delay(5000);
		//Первое измерение с периодом измерения в 1 секунду и сразу фильтрованное.
		sensor_filtr_read(1, DEPTH_FILTRE);
}
/*===========================================================================================================*/
void loop()
{   
	//добавим общую регулярную  перезагрузку  - раз в неделю
	tim = millis();
	if (604800000 - long(tim) <= 0)
	{
		delSMS();
		powerpulse();
		asm volatile ("jmp 0x0000");
	}
	//Если всё хорошо передаём данные на сервер
	if (sendATcommand2("AT+CIFSR", ".", "ERROR", 10000) == 1)
  //if (sendATcommand2(data_ip_protocol, "CONNECT OK", "CONNECT FAIL", 3000) == 1)
		{// ♪♪♪♪♪♪♪♪♪♫ ♫ ♫ Иллюминация правильного состояния модема SIM900 ♪♪♪♪♪♪♪♪
			for (int i = 1; i <= 32; i++)
			{
				ledblink();
			}
		}
		else//иначе  SIM900 был выключен или чтото пошло не так
		{  //Если SIM900 не отзывается на самую простую команду он точно выключен или висит
				if (sendATcommand2("AT", "OK", "OK", 2000) == 0)
				{
						Serial.println("SIM900 OFF..."); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
						power_on();
				}
				Serial.println("Connecting to the GSM network..."); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				while (sendATcommand2("AT+CREG?", "+CREG: 0,1", "+CREG: 0,5", 1000) == 0);
				//настраиваем sim900
				initialize_Gprs();  //организуем UDP соединение с narodmon.ru:8283
		}
		if (sendATcommand2("AT", "OK", "OK", 2000) != 0)
		{
				//Передаём данные на сервер
				send_data();
				//Продолжаем измерение
				sensor_filtr_read(PERIOD, DEPTH_FILTRE);
		}
}
/*===========================================================================================================*/
void  send_data()
{     
	digitalWrite(ledPin, HIGH);   // включаем LED
	//Отключаем эхо
	Serial.println("ATE0");
	//Вычисляем данные для передачи
	calculation();
	/*
	В результате в текстовом буфере- buf_ip_data с размером  strlen(buf_ip_data) байт оказалась подобное этому -
	#112233AACCFF
	#T1#+25.00
	#H1#29.69
	#T2#+25.00
	#P1#754.21
	#Z1#+6.09
	##
	Это и отправим на сервер narodmon.ru*/
	sprintf(aux_str, "AT+CIPSEND=%d", strlen(buf_ip_data)); //Указываем модулю число  байт равное  длине данных в  буфера  buf_ip_data
	if (sendATcommand2(aux_str, ">", "ERROR", 10000) == 1)  //если получили приглашение ">"  передаём buf_ip_data в порт. 
	{
		sendATcommand2(buf_ip_data, "SEND OK", "ERROR", 10000); //Если всё прошло нормально, то в ответ получим "SEND OK"
		delay(500); Serial.println("ATE1"); //Включаем эхо
		/*
		delay(1500);
		Serial.print("\n***********************\n"); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		Serial.print("SEND TO GPRS DATA\n");        //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		Serial.print("in ");                        //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		Serial.println(data_ip_protocol);           //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		Serial.print(strlen(buf_ip_data));              //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		Serial.print(" - BYTE\n\n");                //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		Serial.print(buf_ip_data);                      //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		Serial.print("\n***********************\n\n\n"); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		*/
	}
	else  //Иначе ошибка - ERROR получен из порта или вообще ничего. Не судьба...
	{
		// Closes the socket
		sendATcommand2("AT+CIPCLOSE", "CLOSE OK", "ERROR", 10000);
		powerpulse();
		Serial.println("SEND TO GPRS FAILED"); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
}
// Closes the socket
// sendATcommand2("AT+CIPCLOSE", "CLOSE OK", "ERROR", 10000);
digitalWrite(ledPin, LOW);    // выключаем LED
}
//===========================================================================================================
void initialize_Gprs()
{
		// Selects Single-connection mode
		if (sendATcommand2("AT+CIPMUX=0", "OK", "ERROR", 5000) == 1)
		{
				// Waits for status IP INITIAL
				while (sendATcommand2("AT+CIPSTATUS", "INITIAL", "", 5000) == 0);
				delay(5000);
				// Sets the APN, user name and password
				if (sendATcommand2(command_APN, "OK", "ERROR", 30000) == 1)
				{
						// Waits for status IP START
						while (sendATcommand2("AT+CIPSTATUS", "START", "", 5000) == 0);
						// Brings Up Wireless Connection
						if (sendATcommand2("AT+CIICR", "OK", "ERROR", 30000) == 1)
						{
								// Waits for status IP GPRSACT
								while (sendATcommand2("AT+CIPSTATUS", "GPRSACT", "", 5000) == 0);
								// Gets Local IP Address
								if (sendATcommand2("AT+CIFSR", ".", "ERROR", 15000) == 1)
								{
										// Waits for status IP STATUS
										237
										while (sendATcommand2("AT+CIPSTATUS", "IP STATUS", "", 5000) == 0);
									238
										//delay(5000);
										239
										Serial.println("Openning TCP/UDP"); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
									240

										241
										if (sendATcommand2(data_ip_protocol, "CONNECT OK", "CONNECT FAIL", 30000) == 1)
											242
											//data_ip_protocol = "AT+CIPSTART=\"UDP\",\"IP_address\",\"port\""
											243
										{
											244
												return;
											245
												// Closes the socket
												246
												//sendATcommand2("AT+CIPCLOSE", "CLOSE OK", "ERROR", 10000);
												247
										}
									248
										else
										249
										{
											250
												Serial.println("Error openning the connection"); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
											251
										} // Opens a TCP socket
									252
								}
							253
								else
								254
								{
									255
										Serial.println("Error getting the IP address"); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
									256
								}
							257
						}
					258
						else
						259
						{
							260
								Serial.println("Error bring up wireless connection"); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
							261
						}
					262
				}
			263
				else
				264
				{
					265
						Serial.println("Error setting the APN"); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
					266
				}
			267
		}
	268
		else
		269
		{
			270
				Serial.println("Error setting the single connection"); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
			271
		}
	272
		sendATcommand2("AT+CIPSHUT", "OK", "ERROR", 10000);
	273
		powerpulse(); //Выключаем sim900
	274
}
/*===========================================================================================================*/
void power_on()
{   
	//Включаем sim900
	Serial.println("Starting  SIM900-ON..."); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
	uint8_t answer = 0;
	// checks if the module is started
	while (answer == 0)// waits for an answer from the module
	{    // power on pulse
			powerpulse();
			//если не понимает "AT" -  включаем правильную скорость модема
			if (sendATcommand2("AT", "OK", "OK", 2000) != 1) bruteforce_speed();
			answer = sendATcommand2("AT", "OK", "OK", 2000);
	} //переинициируем датчики на всякий случай.
	dht.begin(); bmp.begin();
	delSMS();
}
/*===========================================================================================================*/
void  powerpulse()
{      
	digitalWrite(onSIM900Pin, HIGH);
	delay(1500);
	digitalWrite(onSIM900Pin, LOW);
	delay(10000);
}
/*===========================================================================================================*/
void bruteforce_speed()
{	/* Было так, что пришел модуль с  запрограммированной скоростью 4800 пока допетрил много чая выпито было.
	* так вот, что-бы подобное  не повторялось - бъём хряка  в лоб кувалдой.
	* Иными словами перебираем все скорости и шлём в порт "парле ву франсэ" (команду включения  нужной SPEED_RXTX)
	* по албански , на корякском и других немецких.   */
	long int speedBF[] = { 1200,2400,4800,9600,19200,38400,57600,115200,SPEED_RXTX };
	for (int i = 0; i < 9; i++)
	{
		Serial.begin(speedBF[i]);
		Serial.print("AT+IPR="); Serial.println(SPEED_RXTX);
		delay(500);
		ledblink();
	}
}

/*******************************************************************************************************
* Этой Функйии передаётся 4 параметра
* 1- AT-команда для отсылкм в порт
* 2- Два возможных ответа
* 3- время ожидания ответа на команду
* Возвращает 0 если не долждались того, что ждали  за время timeout или 1 или 2 в зависимости от ответа.
* использует в памяти  100 байтовый буфер.
*******************************************************************************************************/
int8_t sendATcommand2(char* commandAT, char* expected_answer1, char* expected_answer2, unsigned int timeout)
{
		uint8_t x = 0, answer = 0; // возвращаемый параметр
		char response[100]; // размер приёмного буфера
		unsigned long previous; //переменная для хранения времени начала операции
		memset(response, '\0', 100);    // Initialize the string
		delay(100);
		while (Serial.available() > 0) Serial.read();    // Clean the input buffer
		Serial.println(commandAT);    // Send the AT command
		x = 0;
		previous = millis(); //синхронизируем часы
		// this loop waits for the answer
		do {
			// if there are data in the UART input buffer, reads it and checks for the asnwer
			if (Serial.available() != 0)
			{ 
				response[x] = Serial.read();
				x++;
				// check if the desired answer 1  is in the response of the module
				if (strstr(response, expected_answer1) != NULL)
				{
					answer = 1;
				}
				// check if the desired answer 2 is in the response of the module
				else if (strstr(response, expected_answer2) != NULL)
				{
					answer = 2;
				}
			}
		}
		// Waits for the asnwer with time out
		while ((answer == 0) && ((millis() - previous) < timeout));
		return answer;
}
/*===========================================================================================================*/
/****************************************************
В цикле равном глубине фильтрации опрашивем датчик
находим среднее арифметическое за период измерений
*****************************************************/
void sensor_filtr_read(unsigned long timer, int ftr)
{
		Serial.print("Connecting to DHT22 period  =  "); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		Serial.print(timer);                            //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		Serial.println("s.");                           //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		//Pressure_f     = 0;
		//Temperature_f = 0;
		temperature_f = 0;
		humidity_f = 0;
		int Q = 0;
		tempeBMP085_f = 0;
		pressBMP085_f = 0;
		int W = 0;
		for (int i = 1; i <= ftr; i++)
		{
			delayLED(timer);
			// Reading temperature or humidity takes about 250 milliseconds!
			// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
			digitalWrite(ledPin, HIGH);   // включаем LED
			delay(448);
			float h = dht.readHumidity();
			delay(448);
			float t = dht.readTemperature();
			digitalWrite(ledPin, LOW);    // выключаем LED
			if (isnan(t) || isnan(h))
			{ //Err++;
				Serial.println("  Failed to read from DHT "); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				
			}
			else
			{
				//temperatureTMP = t; //ПОЛУЧАЕМ DHT22 ТЕМПЕРАТУРУ
				temperature_f = temperature_f + t;
				//humidity_fTMP = h; //ПОЛУЧАЕМ DHT22 ВЛАЖНОСТЬ
				humidity_f = humidity_f + h;
				Q++;
				Serial.print("     DHT "); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.print(t);          //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.print(" C");       //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.print("  ---  ");  //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.print(h);          //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.print("%  ");      //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.println(Q);        //Для удобства наладки дублируем на терминал(надо потом закоментировать)   
			}
			if (!bmp.begin())
			{
				Serial.println("Could not find a valid BMP085 sensor, check wiring!"); //Для удобства наладки дублируем на терминал(надо потом закоментировать)  
			}
			else
			{
				W++;
				float t = bmp.readTemperature();
				tempeBMP085_f = tempeBMP085_f + t;
				float P = bmp.readPressure();
				pressBMP085_f = pressBMP085_f + P;
				Serial.print("     BMP "); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.print(t);           //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.print(" C");        //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.print("  ---  ");   //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.print(P);           //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.print("Pa  ");      //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.print(P / 133.3);     //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.print(" MM ");      //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				Serial.println(W);         //Для удобства наладки дублируем на терминал(надо потом закоментировать)        
			}
		}
		if (Q == 0)
		{
				/*Если датчик DHT вообще неопрашивается - отправляем "отфонарные" значения,
				по которым определим что есть неисправность датчика*/
				temperatureTMP = -0.01;
				// Serial.println(temperatureTMP); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				humidity_fTMP = -1;
				// Serial.println(humidity_fTMP);    //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		}
		else
		{ //среднее показание за период DHT
				temperatureTMP = temperature_f / Q;
				humidity_fTMP = humidity_f / Q;
				//  Serial.println(temperatureTMP,3); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				// Serial.println(humidity_fTMP,3); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		}
		if (W == 0)
		{
				/** если датчик BMP085 вообще неопрашивается - отправляем "отфонарные" значения,
				по которым определим что есть неисправность датчика*/
				tempeBMP085TMP = -0.01;
				// Serial.println(tempeBMP085TMP,3); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				pressBMP085TMP = -1;
				// Serial.println(pressBMP085TMP,3); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		}
		else
		{
				//среднее показание за период BMP085
				tempeBMP085TMP = tempeBMP085_f / W;
				pressBMP085TMP = pressBMP085_f / W;
				// Serial.println(tempeBMP085TMP,3); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				// Serial.println(pressBMP085TMP,3); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				pressBMP085_f = pressBMP085_f / W;
				pressBMP085TMP = pressBMP085TMP / 133.3;
				// Serial.println(pressBMP085TMP,3); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
				// Serial.println(pressBMP085TMP/133.3); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
		}
}
/*===========================================================================================================*/
void calculation()
{
		char z = '+';
		if (0 > temperatureTMP)  z = '-';
		char zBMP = '+';
		if (0 > tempeBMP085TMP)  zBMP = '-';
		//Форматируем  температуру с DHT22 на вывод
		float tDHT = abs(temperatureTMP);
		int temptDHT = (tDHT - int(tDHT)) * 10; // выделяем дробную часть десятые
		int tempTtDHT = (tDHT - int(tDHT)) * 100 - temptDHT * 10; // выделяем дробную часть сотые
		//Форматируем влажность с DHT22 на вывод                   
		float humidityDHT = float(humidity_fTMP);
		int temp_H_humidityDHT = (humidityDHT - int(humidityDHT)) * 10; // выделяем дробную часть десятые
		int temp_L_humidityDHT = (humidityDHT - int(humidityDHT)) * 100 - temp_H_humidityDHT * 10; // выделяем дробную часть сотые
		//Форматируем  температуру с BMP085 на вывод
		float tBMP = abs(tempeBMP085TMP);
		int temptBMP = (tBMP - int(tBMP)) * 10; // выделяем дробную часть десятые
		int tempttBMP = (tBMP - int(tBMP)) * 100 - temptBMP * 10; // выделяем дробную часть сотые
		//Форматируем  Давление с BMP085 на вывод
		float pBMP = pressBMP085TMP;
		int tempPBMP = (pBMP - int(pBMP)) * 10; // выделяем дробную часть десятые
		int tempPPBMP = (pBMP - int(pBMP)) * 100 - tempPBMP * 10; // выделяем дробную часть сотые
		//Форматируем точку росы на вывод                   
		float dpDHT = float(dewPoint(temperatureTMP, humidity_fTMP));
		char zR = '+';
		if (0 > temperatureTMP)  zR = '-';
		dpDHT = abs(dpDHT);
		int tempdpDHT = (dpDHT - int(dpDHT)) * 10; // выделяем дробную часть десятые
		int tempDPTDHT = (dpDHT - int(dpDHT)) * 100 - tempdpDHT * 10; // выделяем дробную часть сотые
		//Форматируем время работы на вывод
		tim = millis();
		float timems1 = float(tim) / 1000 / 3600;
		int working1 = timems1;
		int working2 = timems1 * 10 - working1 * 10;
		int working3 = timems1 * 100 - working1 * 100 - working2 * 10;
		//Трамбуем в символьный буфер "buf_ip_data" в установленном для отправки на narodmon.ru формате.
		sprintf(buf_ip_data, "#" MAC_ADDRESS "\n" SOURCE_1  "%c%d.%d%d" "\n" SOURCE_2 "%d.%d%d" "\n" SOURCE_3  "%c%d.%d%d" "\n" SOURCE_4 "%d.%d%d" "\n" SOURCE_5  "%c%d.%d%d" "\n" SOURCE_6 "%d.%d%d" "\n" endCODE,
			z, abs(int(tDHT)), abs(temptDHT), abs(tempTtDHT),
			int(humidityDHT), abs(temp_H_humidityDHT), abs(temp_L_humidityDHT),
			zBMP, abs(int(tBMP)), abs(temptBMP), abs(tempttBMP),
			int(pBMP), abs(tempPBMP), abs(tempPPBMP),
			zR, abs(int(dpDHT)), abs(tempdpDHT), abs(tempDPTDHT),
			working1, working2, working3);
		// Serial.print(buf_ip_data); //Для удобства наладки дублируем на терминал(надо потом закоментировать)
}
/*===========================================================================================================*/
void delSMS()
{    // Delete All messages
		//mySerial.println( "AT+CMGF=1" ); // Did not help
		//delay(1000);                      //Did not help
		//Serial.println( "Delete SMS from SIMCARD" );
		Serial.println("AT+CMGDA=\"DEL ALL\"");
		//Serial.println("DEL ALL\"");
		delay(5000);
		Serial.println("AT+CMGD=4");
		delay(5000);
		Serial.println("All Messages Deleted");
}
/*===========================================================================================================*/
void delayLED(unsigned long cicle)
{
		for (int i = 1; i <= cicle; i++)
		{
				digitalWrite(ledPin, LOW);
				delay(450);
				Serial.print("*");
				digitalWrite(ledPin, HIGH);
				delay(100);
				digitalWrite(ledPin, LOW);
				delay(450);
		}
		Serial.print("\n");
}
/*===========================================================================================================*/
// dewPoint function NOAA
//Эмпирическая формула получения точки росы длинная
// reference: <a href="http://wahiduddin.net/calc/density_algorithms.htm" title="http://wahiduddin.net/calc/density_algorithms.htm" rel="nofollow">http://wahiduddin.net/calc/density_algorithms.htm</a>
double dewPoint(double celsius, double humidity)
{
		double A0 = 373.15 / (273.15 + celsius);
		double SUM = -7.90298 * (A0 - 1);
		SUM += 5.02808 * log10(A0);
		SUM += -1.3816e-7 * (pow(10, (11.344*(1 - 1 / A0))) - 1);
		SUM += 8.1328e-3 * (pow(10, (-3.49149*(A0 - 1))) - 1);
		SUM += log10(1013.246);
		double VP = pow(10, SUM - 3) * humidity;
		double T = log(VP / 0.61078);   // temp var
		return (241.88 * T) / (17.558 - T);
}
/*===========================================================================================================*/
void ledblink()
{
		digitalWrite(ledPin, HIGH);   // включаем LED
		delay(80);
		digitalWrite(ledPin, LOW);    // выключаем LED
		delay(80);
}
// delta max = 0.6544 wrt dewPoint()
// 5x faster than dewPoint()
/*===========================================================================================================*/
/*
//Эмпирическая формула получения точки росы короткая
// reference: <a href="http://en.wikipedia.org/wiki/Dew_point" title="http://en.wikipedia.org/wiki/Dew_point" rel="nofollow">http://en.wikipedia.org/wiki/Dew_point</a>
double dewPointFast(double celsius, double humidity)
{
double a = 17.271;
double b = 237.7;
double temp = (a * celsius) / (b + celsius) + log(humidity/100);
double Td = (b * temp) / (a - temp);
return Td;
}

void asmreset()
{
asm volatile ("jmp 0x0000");
}
*/
