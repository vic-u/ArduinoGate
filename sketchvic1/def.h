#include <arduino.h>

#ifndef VIC_DEV
	#define VIC_DEV
  
	#define RC_NOTHING 0
	#define RC_UNKNOWN 1
	#define RC_DEVICEOFF 2
	#define RC_DEVICEON 3
  
	#define MAX_LONG 0xFFFFFFFF
	#define PHONENUM "+79160265679"
	//#define PHONENUM2"+79850455595"
	#define PHONENUM2"+79262107898"	
//отправка данных через смс и отправка данных через интернет
	#define VER "4"
	
	#define SSR_1  7
	#define SSR_2  5
	#define SSR_W  6 // 9 цифровой пин для включения подогрева воды
 
	#define THERMROOMIN  3
 
  
	#define MAXROOMTEMP  25 // максимальная температура в комнате
	//#define HIGHBOXTEMP  40 //максимальная темппература в корпусе, после которой включается вентилятор охлаждения
	#define DELTATEMP 2 //гестерезис температуры
	#define  MAC_ADDRESS "26FD52AD4E93"
	//tcp index
	#define  ON 1
	#define OFF 2
	
  #ifndef _TRACE 
   #define _TRACE
  #endif  
const char help_msg[]			PROGMEM	= { "GSM INIT\nSTART ALL\nSTART NO HEAT\nSTOP ALL\nSTOP NO HEAT\nSTOP ONLY HEAT\nSTATUS\nSET TEMP=25 \nSET DELTA=2" };
const char rt_msg[]				PROGMEM = { "ROOM TEMP = " };
const char water_msg[]			PROGMEM = { "WATER " };
const char htr_msg[]			PROGMEM = { "HEAT  " };
const char on_msg[]				PROGMEM = { "ON" };
const char off_msg[]			PROGMEM = { "OFF" };

const char sn_msg[]				PROGMEM = { "\n" };
const char fr_msg[]				PROGMEM = { "FR= " };
const char ver_msg[]			PROGMEM = { " V= " };
const char gsci_msg[]			PROGMEM = { "Get sms command at index: " };

const char cmd_start_all[]		PROGMEM = { "STARTALL\0" };
const char cmd_start_no_heat[]	PROGMEM = { "STARTNOHEAT" };
const char cmd_stop_all[]		PROGMEM = { "STOPALL" };
const char cmd_stop_no_heat[]	PROGMEM = { "STOPNOHEAT" };
const char cmd_stop_only_heat[]	PROGMEM = { "STOPONLYHEAT" };
const char cmd_status[]			PROGMEM = { "STATUS" };
const char cmd_set_temp[]		PROGMEM = { "SETTEMP" };
const char cmd_set_delta[]		PROGMEM = { "SETDELTA" };
const char cmd_help[]			PROGMEM = { "HELP" };


const char ts_msg[]				PROGMEM = { "TEMP SET  "};
const char ds_msg[]				PROGMEM = { "DELTA SET "};

//команды ответа модема
const char mdm_ip_ok[]			PROGMEM = { "." };
const char mdm_error[]			PROGMEM = { "ERROR" };
const char mdm_ok[]				PROGMEM = { "OK" };
const char mdm_initial[]		PROGMEM = { "INITIAL" };
const char mdm_empty[]		    PROGMEM = { "" };
const char mdm_start[]		    PROGMEM = { "START" };
const char mdm_ntw_reg_1[]		PROGMEM = { "+CREG: 0,1" };
const char mdm_ntw_reg_5[]		PROGMEM = { "+CREG: 0,5" };
const char mdm_gprsact[]		PROGMEM = { "GPRSACT" };
const char mdm_ip_status[]		PROGMEM = { "IP STATUS" };
const char mdm_cnct_ok[]		PROGMEM = { "CONNECT OK" };
const char mdm_cnct_fail[]		PROGMEM = { "CONNECT FAIL" };
const char mdm_arrow[]			PROGMEM = { ">" };
const char mdm_send_ok[]		PROGMEM = { "SEND OK" };
const char mdm_close_ok[]		PROGMEM = { "CLOSE OK"};
const char mdm_sms_list[]		PROGMEM = { "+CMGL:"};

const char tmpl_sms[]			PROGMEM = { "+CMTI:" };
const char tmpl_tcp[]			PROGMEM = { "+CIPRXGET:" };

const char rest_h1[]			PROGMEM = { "GET /entry/ " };
const char rest_h2[]			PROGMEM = { " HTTP/1.1\r\n" };
const char rest_h3[]			PROGMEM = { "Host:ardu.damasarent.com\r\n" };
const char rest_h4[]			PROGMEM = { "User-Agent:ARDU\r\n" };
const char rest_h5[]			PROGMEM = { "Accept:text/html\r\n" };
const char rest_h6[]			PROGMEM = { "Connection:close\r\n" };
const char rest_h7[]			PROGMEM = { "\r\n" };
const char comma[]				PROGMEM = { "," };

class SSR
{
public:
  SSR(){};
  ~SSR(){};
  void Init(){};
  void On()
  {
    
  };
  void Off() {
    digitalWrite(SSR_1, HIGH);
    delay(500);
	digitalWrite(SSR_2, LOW);
	delay(500);
    digitalWrite(SSR_W, HIGH);
    delay(500);
  };
  void Blink() { // когда все хорошо 5 морганий
	  int led = 13;
	  pinMode(led, OUTPUT);
	  for (int i = 0; i < 5; i++)
	  {
		  digitalWrite(led, HIGH); // turn the LED on (HIGH is the voltage level)
		  delay(1000);
		  digitalWrite(led, LOW); // turn the LED off by making the voltage LOW
		  delay(1000);
	  }
  }
  void ErrorBlink() {//когда плохо, то три долгих
	  int led = 13;
	  pinMode(led, OUTPUT);
	  for (int i = 0; i < 3; i++)
	  {
		  digitalWrite(led, HIGH); // turn the LED on (HIGH is the voltage level)
		  delay(3000);
		  digitalWrite(led, LOW); // turn the LED off by making the voltage LOW
		  delay(3000);
	  }
  }
  int freeRam() {
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
  }

};
#endif