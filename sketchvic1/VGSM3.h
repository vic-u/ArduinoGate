#pragma once
#include <SoftwareSerial.h>
#include "def.h"
#include "water.h"
#include "heat.h"
#include "mylcd.h"

//команда - конфигурации модема под оператора MTS
//#define command_APN "AT+CSTT=\"internet.mts.ru\",\"mts\",\"mts\""
//команда - конфигурации модема под оператора beeline
#define command_APN "AT+CSTT=\"\",\"beeline\",\"beeline\""
//команда - конфигурации модема под оператора МегаФон
//#define command_APN "AT+CSTT=\"internet\",\"gdata\",\"gdata\""
//команда - конфигурации модема под оператора Теле2
//#define command_APN "AT+CSTT=\"internet.tele2.ru\",\"\",\"\""
#define data_ip_protocol "AT+CIPSTART=\"UDP\",\"narodmon.ru\",\"8283\""
 

class VGSM3
{
private:
	SoftwareSerial GSMport = SoftwareSerial(2, 3); // открываем работу с gsm модулем через программный эмулятор сериал через пин 2 и 3
	
public:
	int sms_index = 0;
	int tcp_index = 0;
	char serial_buff[300];
	char aux_str[20];
	char out_msg_buff[300];
	char out_phn_buff[13];
	char in_msg_buff[15];
	char buf_ip_data[150];
	char last_tcp_command[4];

	VGSM3();
	~VGSM3();
	void InitGSM(MYLCD& lcd);
	int8_t sendATcommand4_P(const __FlashStringHelper * commandAT, const char* expected_answer1, const char* expected_answer2, unsigned int timeout, unsigned int adelay = 1000);
	int8_t sendATcommand4(const char* commandAT, const char* expected_answer1, const char* expected_answer2, unsigned int timeout, unsigned int adelay = 1000);
	void ReadBuffer(unsigned int timeout = 10000);
	boolean Reset();
	boolean InitializeGprs();
	
	boolean SendATCommand();
	int TCPSocketResponse();
	boolean CheckTCPCommand(Heater& htr, Water& wtr);
	int NewSMS();
	void DeleteSpaceAndUpper();
	void SendInitSMSChr();
	void SendIndexSMSChr();
	void SendSMSChr(char text[], char phone[]);
	void ReadSMS();
	void ReadTCP();
	boolean CheckSMSCommand(Heater& htr, Water& wtr);
	void StatusChr(double roomtemp, boolean wtrflag, boolean htrflag, int free_ram);
	boolean ConvertTempChr(char * command, int &t);
	boolean send_data2();
};

