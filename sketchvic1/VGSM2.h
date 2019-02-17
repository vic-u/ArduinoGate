#pragma once
#include <arduino.h>
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>
#include<HardwareSerial.h>
#include "mylcd.h"
#include "heat.h"
#include "water.h"

class VGSM2
{
private:
	SoftwareSerial GSMport = SoftwareSerial(2, 3); // открываем работу с gsm модулем через программный эмулятор сериал через пин 2 и 3
public:
	char serial_buff[300];
	char *p_serial_buff;
	
	char out_msg_buff[300];
	char *p_out_msg_buff;

	char out_phn_buff[13];
	char *p_out_phn_buff;

	char in_msg_buff[15];
	char *p_in_msg_buff;

	int index = 0;
	VGSM2();
	~VGSM2();
	void InitGSM(MYLCD& lcd);
	boolean SendATCommand();
	int NewSMS();
	void ReadSMS();
	void Reset();
	boolean CheckSMSCommand(Heater& htr, Water& wtr);
	void ClearBuffer(); //заполняем буффер ответа от модема нулями
	void ClearOutMsgBuffer();//заполняем буфет отправки сообщения нулями
	void ClearOutPhnBuffer();//заполняем буфет номера телефона нулями
	void ClearInMsgBuffer();
	void ClearMemoryBuffer(char memory_buffer[]);
	void ReadBuffer();
	void DeleteSpaceAndUpper();

	void SendInitSMSChr();
	void SendSMSChr(char text[], char phone[]);
	void StatusChr(double roomtemp, boolean wtrflag, boolean htrflag, int free_ram);
	boolean ConvertTempChr(char * command, int &t);
};

