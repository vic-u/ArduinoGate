#include "VGSM3.h"

VGSM3::VGSM3()
{
}
VGSM3::~VGSM3()
{
}
/**
   ������� ��������� ���� ������ �� �������� 9600
   � �������� ������� ��������� �� LCD ������
*/
void VGSM3::InitGSM(MYLCD& lcd) {
#ifdef _TRACE
	Serial.println(F("InitGSM"));
#endif
	GSMport.begin(9600); //��������� ���� ������ �� �������� 9600
#ifdef _TRACE
	Serial.println(F("InitGSM"));
#endif
	lcd.gsmInit();
}
void VGSM3::InitGSM2(void ( *f)()) {
#ifdef _TRACE
	Serial.println(F("InitGSM"));
#endif
	GSMport.begin(9600); //��������� ���� ������ �� �������� 9600
#ifdef _TRACE
	Serial.println(F("InitGSM"));
#endif
	if (f !=NULL) f();
}
/**
   ���������� � ����� ������� �� commandAT, ������� ��������� � ������ �������
   ������ ��������(expected_answer1) ��� ������� (expected_answer2) ������, ��������� 1 ��� 2 ��������������
   ���������� ��������� ������ ������ � ���������� ����� serial_buff
*/
int8_t VGSM3::SendATcommand4_P(const __FlashStringHelper *commandAT, const char* expected_answer1, const char* expected_answer2, unsigned int timeout, unsigned int adelay)
{
	GSMport.println(commandAT);  // ����������� ��������� ��� �� ��� �����  � ������ �������
	delay(adelay);
	ReadBuffer(timeout); //������� ����� ������ � ������� � �������
#ifdef _TRACE
	Serial.println("-senATcommand4->");
	Serial.println(serial_buff);
	//Serial.println((const __FlashStringHelper *)expected_answer1);
	//Serial.println((const __FlashStringHelper *)expected_answer2);
#endif
	/*Serial.println(F(">>"));
	while (!Serial.available()) {};
	Serial.read();*/

	if (strstr_P(serial_buff, expected_answer1) != NULL) {
		/*Serial.println(F(">>"));
		while (!Serial.available()) {};
		Serial.read();*/
		return 1;
	}
	if (strstr_P(serial_buff, expected_answer2) != NULL) return 2;
	return 0;
}
/**
   ���������� � ����� ������� �� commandAT
   ������ ��������(expected_answer1) ��� ������� (expected_answer2) ������, ��������� 1 ��� 2 ��������������
   ���������� ��������� ������ ������ � ���������� ����� serial_buff
*/
int8_t VGSM3::SendATcommand4(const char* commandAT, const char* expected_answer1, const char* expected_answer2, unsigned int timeout, unsigned int adelay) {
	GSMport.println(commandAT);  // ���������� ������� � ���� ������
	delay(adelay);
	ReadBuffer(timeout); //������� ����� ������ � ������� � �������
#ifdef _TRACE
	Serial.println("-senATcommand4->");
	Serial.println(serial_buff);
	//Serial.println((const __FlashStringHelper *)expected_answer1);
	//Serial.println((const __FlashStringHelper *)expected_answer2);
#endif
	if (strstr_P(serial_buff, expected_answer1) != NULL) return 1;
	if (strstr_P(serial_buff, expected_answer2) != NULL) return 2;
	return 0;
}
/**
   ������� ������ ������ �� ������ ������ � ���������� ����������
   serial_buff. � ���� timeout ������� ���� �� �������� �������� ���� ������
*/
void VGSM3::ReadBuffer(unsigned int timeout = 10000) {
	unsigned long previous = millis();
	memset(serial_buff, '\0', sizeof(serial_buff));
	//Serial.println("-resetBuffer->");
	//Serial.println(serial_buff);
	int count = 0; //������� ����������� �� ������� ������, ����� �� ���� �� ������� �������
	char empty; //������ ����������, � ������� ����� ������, ���� ����� ����������, � ���� ��� ����� ����� ������

	while (GSMport.available() && ((millis() - previous) < timeout)) {  //���� �� �������� ������ � ����� ��� �� ������� ����� �������� ������ �� � ������ 	
		if (count <= sizeof(serial_buff)) { //���� �� ������ ������, ������ � ����
			serial_buff[count] = GSMport.read();
			count++; // ����������� ������� ����� �����, ����� �� �������� ��� ���������
		}
		else {
			empty = GSMport.read(); //������ ������ � ������ ����������, ���� �� �� ��������
		}
		delay(10);
	}
	DeleteSpaceAndUpper();//������� ������� � ��������� �����
	//Serial.println(serial_buff);
	//NewSMS(); //��������� ������ � ������� �� ������� ������� � ������� ����� ���, ���� ������, ������� �� ������ ��� ������ ������
}
/**
   ������� ���������� �����, ������������� �� ��������� ��� � �������� � ���������� ������
   � ������� �� ������ ��� ���
*/
boolean VGSM3::Reset() {
#ifdef _TRACE
	Serial.println(F("Send Reset"));
#endif
	//delay(5000);
	if(SendATcommand4_P(F("AT+CFUN=1,1"), mdm_ok, mdm_error, 10000, 20000) != 1) return false;//������� ������������ ������ ���������� � ���� ������ // ���� 10 ������
	

#ifdef _TRACE
	Serial.println(serial_buff);
	Serial.println("1");
#endif
	if (SendATcommand4_P(F("AT"), mdm_ok, mdm_error, 2000) != 1) return false;//������ ������� � �����, ��� ��������, ��� ��������� ����� ������������// ���� 10 ������ ����� ��� ������ ����� ����� � �����

#ifdef _TRACE
	Serial.println(serial_buff);
	Serial.println("1.1");
#endif
	/*Serial.println(F(">>"));
	while (!Serial.available()) {};
	Serial.read();*/

	if (SendATcommand4_P(F("AT+CPMS= \"MT\""), mdm_ok, mdm_error, 2000) != 1) return false; // ����������� ��������� ��� �� ��� ����� � �������
#ifdef _TRACE
	Serial.println(serial_buff);
	Serial.println("3");
#endif
	//memset(aux_str, '\0', sizeof(aux_str));
	for (int i = 1; i < 11; i++) { // ������� ��������� 
		SMSDelete(i);
//		sprintf(aux_str, "AT+CMGD=%d,0\0", i);
//		SendATcommand4(aux_str, mdm_ok, mdm_error, 2000);
//#ifdef _TRACE
//		//Serial.println(serial_buff);
//#endif
	}
	return true;
}
boolean VGSM3::InitializeGprs() {
// http://badembed.ru/sim900-tcp-soedinenie-s-serverom/
	//� ������ ������ �� ���� �� ������, ������ � ������������ 
	// Selects Single-connection mode
	if (SendATcommand4_P(F("AT+CIPMUX=0"), mdm_ok, mdm_error, 5000) != 1) return false;
	// Waits for status IP INITIAL
	while (SendATcommand4_P(F("AT+CIPSTATUS"), mdm_initial, mdm_empty, 5000) == 0);
	delay(5000);
	// Sets the APN, user name and password
	if (SendATcommand4_P(F(command_APN), mdm_ok, mdm_error, 30000) != 1) return false;
	// Waits for status IP START
	while (SendATcommand4_P(F("AT+CIPSTATUS"), mdm_start, mdm_empty, 5000) == 0);
	// Brings Up Wireless Connection
	if (SendATcommand4_P(F("AT+CIICR"), mdm_ok, mdm_error, 30000) != 1) return false;
	// Waits for status IP GPRSACT
	while (SendATcommand4_P(F("AT+CIPSTATUS"), mdm_gprsact, mdm_empty, 5000) == 0);
	// Gets Local IP Address
	if (SendATcommand4_P(F("AT+CIFSR"), mdm_ip_ok, mdm_error, 15000) != 1) return false;
	// Waits for status IP STATUS
	while (SendATcommand4_P(F("AT+CIPSTATUS"), mdm_ip_status, mdm_empty, 5000) == 0);
	//delay(5000);
#ifdef _TRACE
	Serial.println(F("Openning TCP/UDP")); //��� �������� ������� ��������� �� ��������(���� ����� ���������������)
#endif
	if (SendATcommand4_P(F(data_ip_protocol), mdm_cnct_ok, mdm_cnct_fail, 1000, 30000) != 1) {

		Serial.println(serial_buff);
		/*Serial.println(F(">>"));
		while (!Serial.available()) {};
		Serial.read()*/;
		return false;
	}
#ifdef _TRACE
	Serial.println(F("GPRS OK"));
#endif
	return true;
}
//boolean VGSM3::SendATCommand() {
//#ifdef _TRACE
//	Serial.println(F("SendATCommand"));
//#endif
//	if (sendATcommand4_P(F("AT"), mdm_ok, mdm_error, 2000, 1000) != 1) return false; //���������� � �������� ���� � ����� ������� �������� 
//
//	return true; //���������� ����� � ���, ��� ����� �����������, ����� ���������� ������� � �������
//	//???���������, ��� �����, ���� ����� �� �����������. ������ ������ ����� ���� ������ � ��� ��������
//}
int VGSM3::TCPSocketResponse() {
	char reply[3];
	if (ParseTemplateChr("=" MAC_ADDRESS, "=", reply)) {
		tcp_index = OFF;
		if (strstr(reply, "ON")) tcp_index = ON;
	}


	//if (strstr(serial_buff, "=" MAC_ADDRESS) == NULL) return 0; //���� ������� ������ �� ������� � ������ ����������� ����� ��������
	//int first = 0; 
	//int second = 0;
	//first = (strstr(serial_buff, "=" MAC_ADDRESS)- serial_buff);
	//second = strlen("=" MAC_ADDRESS "=T1=");
	//char reply[3];
	//memset(reply, '\0', sizeof(reply));
	////ClearMemoryBuffer(reply);
	//int last = (strstr(serial_buff + first + second, "=")- serial_buff);
	//int len = (last - first - 1);
	//strncpy(reply, serial_buff + first + second, len);
	//tcp_index = OFF;
	//if (strstr(reply, "ON")) tcp_index = ON;
	////if (strstr(reply, "OFF")) tcp_index = OFF;
}
//int VGSM3::NewSMS() {
//#ifdef _TRACE
//	Serial.println(F("NewSMS"));
//#endif
//	boolean smsflag, tcpflag = false;
//	//Serial.println(buffer);
//	//char cmd[] = "+CMTI:";// \"ME\","; //������ ������� � ������� ������ ����� ���������
//	DeleteSpaceAndUpper();//������� ������� � ��������� �������
//#ifdef _TRACE
//	Serial.println(serial_buff);
//#endif
//	//��������� ������� � ������ ������� ������ ���������
//	if (strstr_P(serial_buff, tmpl_sms) != NULL) smsflag = true;
//	if (strstr_P(serial_buff, tmpl_tcp) == NULL) tcpflag = true;
//
//	if ((!smsflag) && (!tcpflag)) return 0;//��������� ���, �� � ������ ������ ������
//	//����� ������� ������ ���������
//#ifdef _TRACE
//	Serial.println("get new sms");
//#endif
//	int first = 0;
//	if(tcpflag) first = (strstr_P(serial_buff, tmpl_tcp) - serial_buff) + strlen_P(tmpl_tcp) + 2;
//	if(smsflag) first = (strstr_P(serial_buff, tmpl_sms) - serial_buff) + strlen_P(tmpl_sms) + 5; //11;// ������� ����� ������ ������ ���������, �� ��� ����� ����� �� �����, ������ ��������� ���������
//#ifdef _TRACE
//	//Serial.println(strlen(cmd));
//	Serial.print(F("ser buf len: "));
//	Serial.print(strlen(serial_buff));
//	Serial.print(F(" first pos: "));
//	Serial.println(first);
//#endif
//	// ������ \n �������� \r\n ���� ���������
//	int last = (strstr(serial_buff + first, "\r\n") != NULL) ? 
//		(strstr(serial_buff + first, "\n") - serial_buff) : //������� ����� ������
//		strlen(serial_buff);
//	Serial.println(last);
//
//	char reply[3];
//	memset(reply, '\0', sizeof(reply));
//	//ClearMemoryBuffer(reply);
//	int sms_ind_len = (last - first - 1);
//	strncpy(reply, serial_buff + first, (sms_ind_len <= 2)?sms_ind_len:2); //�������� ����� ����� ��������� �� ����� ������
//#ifdef _TRACE
//	Serial.println(reply);
//#endif
//	//if (tcpflag) tcp_index = atoi(reply); //��������� ��� � ����� � ������ � ������, ������ ��� ����� ��������� � ��������� �����
//	if (smsflag) sms_index = atoi(reply); //��������� ��� � ����� � ������ � ������, ������ ��� ����� ��������� � ��������� �����
//}
void VGSM3::DeleteSpaceAndUpper()
{
	int i = 0, j = 0;
	while (serial_buff[i] != '\0') //���� �� ������������ ������ ���� �� �������� �����
	{
		if (serial_buff[i] != ' ') //���� ������� ������ � ������ �� ������
		{
			serial_buff[j] = toupper(serial_buff[i]);
			++j;
		}

		++i;
	}

	serial_buff[j] = '\0';
}
void VGSM3::SendInitSMSChr()
{
#ifdef _TRACE
	Serial.println(F("Init SMS Send"));
#endif	

	memset(out_msg_buff, '\0', sizeof(out_msg_buff));
	memset(out_phn_buff, '\0', sizeof(out_phn_buff));
	
	//strcpy_P(out_msg_buff, help_msg);
	sprintf_P(out_msg_buff, PSTR("%S%c"), help_msg, 0x1A);
	strcpy(out_phn_buff, PHONENUM);
#ifdef _TRACE
	Serial.println("5");
	Serial.println(out_msg_buff);
	Serial.println(out_phn_buff);
#endif	
	SendSMSChr(out_msg_buff, out_phn_buff);
	delay(2000);
}
void VGSM3::SendIndexSMSChr()
{
	memset(out_msg_buff, '\0', sizeof(out_msg_buff));
	memset(out_phn_buff, '\0', sizeof(out_phn_buff));

	sprintf_P(out_msg_buff, PSTR("%S%d%�"), gsci_msg, sms_index, 0x1A);
	strcpy(out_phn_buff, PHONENUM);
#ifdef _TRACE
	Serial.println("6");
	Serial.println(out_msg_buff);
	Serial.println(out_phn_buff);
#endif	
	SendSMSChr(out_msg_buff, out_phn_buff);
	delay(2000);
}
void VGSM3::SendSMSChr(char text[], char phone[]) {
#ifdef _TRACE
	Serial.println(F("Send sms char"));
#endif
	if (SendATcommand4_P(F("AT+CMGF=1"), mdm_ok, mdm_error, 2000, 1000) != 1) return;
#ifdef _TRACE
	Serial.print(">cmgf>");
	Serial.println(serial_buff);
#endif
	memset(aux_str, '\0', sizeof(aux_str));
	sprintf(aux_str, "AT+CMGS=\"%s\"", phone);
	SendATcommand4(aux_str, mdm_ok, mdm_error, 2000);
	SendATcommand4(text, mdm_ok, mdm_error, 2000, 3000);
	//GSMport.write(0x1A); // ��������� ������ � ������� �������
						 //GSMport.write(0x0D);
						 //GSMport.write(0x0A);
#ifdef _TRACE
	Serial.print(">cmgs>");
	Serial.println(serial_buff);
#endif
}
/**
   ������� ������ ��������� �� ������ ������ ��
   �������, ��������� � ���������� ����������  sms_index 
*/
void VGSM3::SMSRead() {
	if (sms_index == 0) return;
	if (SendATcommand4_P(F("AT+CMGF=1"), mdm_ok, mdm_error, 2000, 1000) != 1) return; //��������� ����� � ��������� �����
#ifdef _TRACE
	Serial.println(serial_buff);
#endif
	if (SendATcommand4_P(F("AT+CPMS= \"MT\""), mdm_ok, mdm_error, 2000, 1000) != 1) return; //��������� ����� �� ��������� ��������� ������
#ifdef _TRACE
	Serial.println(serial_buff);
	Serial.println(sms_index);
	Serial.println("6");
#endif
	memset(aux_str, '\0', sizeof(aux_str));
	sprintf_P(aux_str, PSTR("AT+CMGR=%d\0"), sms_index); 
	if (SendATcommand4(aux_str, mdm_ok, mdm_error, 2000) != 1) return;
	//DeleteSpaceAndUpper(); //������� �� ����������� ��������� ������� � ��������� �������
#ifdef _TRACE
	Serial.println(serial_buff);
#endif
	memset(in_msg_buff, '\0', sizeof(in_msg_buff));
	char cmd[] = "\r\n"; // �������� �� \r\n  ���� ���������
	ParseTemplateChr(cmd, cmd, in_msg_buff);

	//int first = (strstr(serial_buff, cmd) - serial_buff) + strlen(cmd);//���� ������ ��������� �������� ������
	//first = (strstr(serial_buff + first, cmd) - serial_buff) + strlen(cmd);//���� ������ ��������� �������� ������, ����� ���� ����� ����� ���������
	//int last = (strstr(serial_buff + first, cmd) != NULL) ?
	//	(strstr(serial_buff + first, cmd) - serial_buff) : //������� ����� ������
	//	strlen(serial_buff);


	//strncpy(in_msg_buff, serial_buff + first, ((last - first - 1) > 13) ? 13 : (last - first - 1));
#ifdef _TRACE
	Serial.println("7");
	Serial.println(in_msg_buff);
#endif
	/*memset(aux_str, '\0', sizeof(aux_str));
	sprintf_P(aux_str, PSTR("AT+CMGD=%d,0\0"), sms_index);
	if (SendATcommand4(aux_str, mdm_ok, mdm_error, 2000) != 1) return;
#ifdef _TRACE
	Serial.println(serial_buff);
#endif*/
	SMSDelete(sms_index);
	sms_index = 0;
}
/**
   ������� ������� sms ��������� �� ������ ������
   �� ������ ��������� int index
*/
void VGSM3::SMSDelete(int index) {
	memset(aux_str, '\0', sizeof(aux_str));
	sprintf_P(aux_str, PSTR("AT+CMGD=%d,0\0"), sms_index);
	if (SendATcommand4(aux_str, mdm_ok, mdm_error, 2000) != 1) return;
#ifdef _TRACE
	Serial.println(serial_buff);
#endif
}
//void VGSM3::ReadTCP() {
//	if (tcp_index == 0) return;
//	
//	memset(aux_str, '\0', sizeof(aux_str));
//	sprintf(aux_str, "AT+CIPRXGET=2,%d,%d\0", sms_index, strlen(serial_buff));
//	if (SendATcommand4(aux_str, mdm_ok, mdm_error, 2000) != 1) return;
//	DeleteSpaceAndUpper(); //������� �� ����������� ��������� ������� � ��������� �������
//#ifdef _TRACE
//	Serial.println(serial_buff);
//#endif
//	memset(in_msg_buff, '\0', sizeof(in_msg_buff));
//	char cmd[] = "\r\n";
//	int first = (strstr(serial_buff, cmd) - serial_buff) + strlen(cmd);//���� ������ ��������� �������� ������
//	int last  = (strstr(serial_buff + first, cmd) - serial_buff) + strlen(cmd);//���� ������ ��������� �������� ������, ����� ���� ����� ����� ���������
//	
//	strncpy(in_msg_buff, serial_buff + first, ((last - first - 1) > 13) ? 13 : (last - first - 1));
//#ifdef _TRACE
//	Serial.println("7");
//	Serial.println(in_msg_buff);
//#endif
//	tcp_index = 0;
//}
boolean VGSM3::CheckTCPCommand(Heater& htr, Water& wtr) {
	if (tcp_index == OFF) htr.setCommand(RC_DEVICEOFF);
	if (tcp_index == ON) htr.setCommand(RC_DEVICEON);
	return true;
}

boolean VGSM3::CheckSMSCommand(Heater& htr, Water& wtr)
{
	if (in_msg_buff == "\0") return false;
#ifdef _TRACE
	Serial.println(F("Check SMS Command"));
#endif
	//start all
	if (strstr_P(in_msg_buff, cmd_start_all) != NULL) //start all
	{
		htr.setCommand(RC_DEVICEON);
		//wtr.setCommand(RC_DEVICEON);
		Serial.println(F("START ALL"));
		return true;
	}
	//start no heat
	if (strstr_P(in_msg_buff, cmd_start_no_heat) != NULL) //start no heat
	{
		wtr.setCommand(RC_DEVICEON);
		return true;
	}
	//stop all
	if (strstr_P(in_msg_buff, cmd_stop_all) != NULL) // stop all
	{
		htr.setCommand(RC_DEVICEOFF);
		wtr.setCommand(RC_DEVICEOFF);
		return true;
	}
	// stop no heat
	if (strstr_P(in_msg_buff, cmd_stop_no_heat) != NULL)
	{
		wtr.setCommand(RC_DEVICEOFF);
		return true;
	}
	// stop only heat
	if (strstr_P(in_msg_buff, cmd_stop_only_heat) != NULL)
	{
		htr.setCommand(RC_DEVICEOFF);
		return true;
	}
	//status
	if (strstr_P(in_msg_buff, cmd_status) != NULL)
	{
#ifdef _TRACE
		Serial.println(F("sms status"));
#endif
		return true;
	}
	//set temp
	if (strstr_P(in_msg_buff, cmd_set_temp) != NULL)
	{
		int t = 0;
		if (ConvertTempChr(in_msg_buff, t))
		{
			htr.setMaxRoomTemp(t);
			memset(out_msg_buff, '\0', sizeof(out_msg_buff));
			sprintf_P(out_msg_buff, PSTR("%S%d"), ts_msg, t);
			SendSMSChr(out_msg_buff, out_phn_buff);
		}
#ifdef _TRACE
		Serial.println(F("set temp"));
#endif
		return true;
	}
	//set delta
	if (strstr_P(in_msg_buff, cmd_set_delta) != NULL)
	{
		int t = 0;
		if (ConvertTempChr(in_msg_buff, t))
		{
			htr.setDeltaRoomTemp(t);
			memset(out_msg_buff, '\0', sizeof(out_msg_buff));
			sprintf_P(out_msg_buff, PSTR("%S%d"), ds_msg, t);
			SendSMSChr(out_msg_buff, out_phn_buff);
		}
#ifdef _TRACE
		Serial.println(F("set delta temp"));
#endif
		return true;
	}
	//set help
	if (strstr_P(in_msg_buff, cmd_help) != NULL)
	{
		SendInitSMSChr();
#ifdef _TRACE
		Serial.println(F("sms help"));
#endif
		return true;
	}
	return false; //unknown command
}
void VGSM3::StatusChr(double roomtemp, boolean wtrflag, boolean htrflag, int free_ram)
{
	char str_rt[6];
	Serial.println("StatusChr");
	dtostrf(roomtemp, 4, 2, str_rt);
	str_rt[5] = 0x00;
	Serial.println(str_rt);
	memset(out_msg_buff, '\0', sizeof(out_msg_buff));
	sprintf_P(out_msg_buff, PSTR("%S%s%S%S%S%S%S%S%S%S%d%S%d%s"),
		rt_msg, str_rt, sn_msg,
		water_msg, wtrflag ? on_msg : off_msg, sn_msg,
		htr_msg, htrflag ? on_msg : off_msg, sn_msg,
		fr_msg, free_ram, ver_msg, VER,"\0");
	//sprintf_P(out_msg_buff, PSTR("%S%s"),
	//	rt_msg, str_rt); 
#ifdef _TRACE
	Serial.println(out_msg_buff);
#endif
	SendSMSChr(out_msg_buff, out_phn_buff);
	delay(2000);
}
boolean VGSM3::ConvertTempChr(char * command, int &t)
{
	int i = 0;
	char *istr;
	do
	{
		istr = &command[i++];
		if (istr[0] == '=')
		{
			Serial.println("get eq");
			istr = &command[i++];
			t = atoi(istr);
			if (isnan(t)) return false;
			Serial.println(t);

			return true;
		}
	} while (istr[0]);
}
boolean  VGSM3::TCPSendData2()
{
	//digitalWrite(ledPin, HIGH);   // �������� LED
								  //��������� ���
	Serial.println(F("ATE0"));
	//��������� ������ ��� ��������
	//calculation();
	/*
	� ���������� � ��������� ������- buf_ip_data � ��������  strlen(buf_ip_data) ���� ��������� �������� ����� -
	#112233AACCFF
	#T1#+25.00
	#H1#29.69
	#T2#+25.00
	#P1#754.21
	#Z1#+6.09
	##
	��� � �������� �� ������ narodmon.ru*/
	memset(out_msg_buff, '\0', sizeof(out_msg_buff));
	memset(aux_str, '\0', sizeof(aux_str));
	//sprintf(buf_ip_data, "#" MAC_ADDRESS "\n #T1# 26.05\n #Z1# 1 \n ##");
	//sprintf(buf_ip_data, "=" MAC_ADDRESS "=T1=26.05=");
	//sprintf_P(buf_ip_data, PSTR("%S%d"), gsci_msg, sms_index);

	sprintf_P(out_msg_buff, PSTR("%S%s%S%S%S%S%S%S"), rest_h1, "=" MAC_ADDRESS "=T1=26.05=", rest_h2, rest_h3, rest_h4, rest_h5, rest_h6, rest_h7);

	sprintf(aux_str, "AT+CIPSEND=%d", strlen(out_msg_buff));  //��������� ������ �����  ���� ������  ����� ������ �  ������  buf_ip_data

	if (SendATcommand4(aux_str, mdm_arrow, mdm_error, 10000) == 1)  //���� �������� ����������� ">"  ������� buf_ip_data � ����. 
	{
		if (SendATcommand4(out_msg_buff, mdm_send_ok, mdm_error, 10000) == 1) //���� �� ������ ���������, �� � ����� ������� "SEND OK"
		{
			//��� ���� ���������� ����� �� �������, ����� ���� �������� �������
			TCPSocketResponse();
			delay(500);
			Serial.println(F("ATE1")); //�������� ���
			return true;
		}
	}
	else  //����� ������ - ERROR ������� �� ����� ��� ������ ������. �� ������...
	{
		// Closes the socket
		SendATcommand4_P(F("AT+CIPCLOSE"), mdm_close_ok, mdm_error, 10000);
		//powerpulse();

		Serial.println(F("SEND TO GPRS FAILED")); //��� �������� ������� ��������� �� ��������(���� ����� ���������������)
		return false;
	}
	// Closes the socket
	// sendATcommand2("AT+CIPCLOSE", "CLOSE OK", "ERROR", 10000);
	//digitalWrite(ledPin, LOW);    // ��������� LED
}
void VGSM3::SMSCheckNewMsg() {
	
	if (SendATcommand4_P(F("AT+CMGF=1"), mdm_ok, mdm_error, 2000, 1000) != 1) return; //��������� ����� � ��������� �����
#ifdef _TRACE
	Serial.println(serial_buff);
#endif
	if (SendATcommand4_P(F("AT+CPMS= \"MT\""), mdm_ok, mdm_error, 2000, 1000) != 1) return; //�������� ���������
#ifdef _TRACE
	Serial.println(serial_buff);
	Serial.println("6");
#endif
	if (SendATcommand4_P(F("AT + CMGL = \"REC UNREAD\""), mdm_ok, mdm_error, 20000, 10000) != 1) return; //������ ����� ������������� ���������
	char reply[3];
	//����� �������� ����� ��������� � ������ +CMGL:1,1,76
	if (ParseTemplateChr(mdm_sms_list, comma, reply))  sms_index = atoi(reply);
		
//	if (strstr_P(serial_buff, mdm_sms_list) != NULL) //������� � ������ ������������� ��������� ������ � �������
//	{
//		//����� �������� ����� ��������� � ������ +CMGL:1,1,76
//		int first = 0; 
//		first = (strstr_P(serial_buff, mdm_sms_list) - serial_buff) + strlen_P(mdm_sms_list); // ���� ������ ��������� ������� +CMGL:, �� ��� ����� ����� �� ����� 
//		//������� �������������� ���������, ������ ��������� ���������
//
//		// ������ \n �������� \r\n ���� ���������
//		int last = (strstr(serial_buff + first, ",") - serial_buff);
//#ifdef _TRACE
//		//Serial.println(strlen(cmd));
//		Serial.print(F("ser buf len: "));
//		Serial.print(strlen(serial_buff));
//		Serial.print(F(" first pos: "));
//		Serial.println(first);
//		Serial.print(F(" last pos: "));
//		Serial.println(last);
//#endif
//		int len = (last - first - 1);
//		char reply[3];
//		memset(reply, '\0', sizeof(reply));		
//		strncpy(reply, serial_buff + first, len); //�������� ����� ����� ��������� �� ����� ������
//#ifdef _TRACE
//		Serial.println(reply);
//#endif
//		//if (tcpflag) tcp_index = atoi(reply); //��������� ��� � ����� � ������ � ������, ������ ��� ����� ��������� � ��������� �����
//		sms_index = atoi(reply); //��������� ��� � ����� � ������ � ������, ������ ��� ����� ��������� � ��������� �����
	////}
}
//������� �������� ������� ������ �� ����� �������������� �������, �� ������� ����������� ��� ���� ������
boolean VGSM3::ParseTemplateChr(const char *tmpl, const char *delim, char* reply) {
	//���� ������ ��������� ������� � ������

	int first = (strstr_P(serial_buff, tmpl) - serial_buff) + strlen_P(tmpl); // ���� ������ ��������� ������� 
	if (first == NULL) return false;
	// ������ \n �������� \r\n ���� ���������
	int last = (strstr(serial_buff + first, delim) - serial_buff);
	if (last == NULL) return false;
#ifdef _TRACE
	//Serial.println(strlen(cmd));
	Serial.print(F("ser buf len: "));
	Serial.print(strlen(serial_buff));
	Serial.print(F(" first pos: "));
	Serial.println(first);
	Serial.print(F(" last pos: "));
	Serial.println(last);
#endif
	int len = (last - first - 1); //����� ����������� ����� ������
	if (sizeof(reply) <= len) len = sizeof(reply - 1); // ���� ������ ������ ������ ����� ������, �� ��������� � ���� ����� � ��������� 0
	memset(reply, '\0', sizeof(reply));
	strncpy(reply, serial_buff + first, len); //�������� ������
	return true;
}