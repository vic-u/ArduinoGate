#include "VGSM3.h"

VGSM3::VGSM3()
{
}
VGSM3::~VGSM3()
{
}
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
int8_t VGSM3::sendATcommand4_P(const __FlashStringHelper *commandAT, const char* expected_answer1, const char* expected_answer2, unsigned int timeout, 
	unsigned int adelay)
{
	GSMport.println(commandAT);  // ����������� ��������� ��� �� ��� �����  � ������ ��������
	delay(adelay);
	ReadBuffer(timeout); //������� ����� ������ � ������� � �������
#ifdef _TRACE
	Serial.println("-senATcommand4->");
	Serial.println(serial_buff);
	Serial.println((const __FlashStringHelper *)expected_answer1);
	Serial.println((const __FlashStringHelper *)expected_answer2);
#endif
	if (strstr_P(serial_buff, expected_answer1) != NULL) return 1;
	if (strstr_P(serial_buff, expected_answer2) != NULL) return 2;
	return 0;
}
int8_t VGSM3::sendATcommand4(const char* commandAT, const char* expected_answer1, const char* expected_answer2, unsigned int timeout, unsigned int adelay) {
	GSMport.println(commandAT);  // ���������� ������� � ���� ������
	delay(adelay);
	ReadBuffer(timeout); //������� ����� ������ � ������� � �������
#ifdef _TRACE
	Serial.println("-senATcommand4->");
	Serial.println(serial_buff);
	Serial.println((const __FlashStringHelper *)expected_answer1);
	Serial.println((const __FlashStringHelper *)expected_answer2);
#endif
	if (strstr_P(serial_buff, expected_answer1) != NULL) return 1;
	if (strstr_P(serial_buff, expected_answer2) != NULL) return 2;
	return 0;
}
void VGSM3::ReadBuffer(unsigned int timeout = 10000) {
	unsigned long previous = millis();
	memset(serial_buff, '\0', sizeof(serial_buff));

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
}
boolean VGSM3::Reset() {
#ifdef _TRACE
	Serial.println(F("Send Reset"));
#endif	
	if(sendATcommand4_P(F("AT+CFUN=1,1"), mdm_ok, mdm_error, 20000, 10000) != 1) return false;//������� ������������ ������ ���������� � ���� ������ // ���� 10 ������
#ifdef _TRACE
	Serial.println(serial_buff);
	Serial.println("1");
#endif
	if (sendATcommand4_P(F("AT"), mdm_ok, mdm_error, 20000, 10000) != 1) return false;//������ ������� � �����, ��� ��������, ��� ��������� ����� ������������// ���� 10 ������ ����� ��� ������ ����� ����� � �����
#ifdef _TRACE
	Serial.println(serial_buff);
	Serial.println("1.1");
#endif
	if (sendATcommand4_P(F("AT+CPMS= \"MT\""), mdm_ok, mdm_error, 20000, 10000) != 1) return false; // ����������� ��������� ��� �� ��� ����� � �������
#ifdef _TRACE
	Serial.println(serial_buff);
	Serial.println("3");
#endif
	memset(aux_str, '\0', sizeof(aux_str));
	for (int i = 1; i < 11; i++) { // ������� ��������� 
		sprintf(aux_str, "AT+CMGD=%d,0\0", i);
		sendATcommand4(aux_str, mdm_ok, mdm_error, 2000);
#ifdef _TRACE
		Serial.println(serial_buff);
#endif
	}
	return true;
}
boolean VGSM3::InitializeGprs() {
// http://badembed.ru/sim900-tcp-soedinenie-s-serverom/
	//� ������ ������ �� ���� �� ������, ������ � ������������ 
	// Selects Single-connection mode
	if (sendATcommand4_P(F("AT+CIPMUX=0"), mdm_ok, mdm_error, 5000) != 1) return false;
	// Waits for status IP INITIAL
	while (sendATcommand4_P(F("AT+CIPSTATUS"), mdm_initial, mdm_empty, 5000) == 0);
	delay(5000);
	// Sets the APN, user name and password
	if (sendATcommand4_P(F(command_APN), mdm_ok, mdm_error, 30000) != 1) return false;
	// Waits for status IP START
	while (sendATcommand4_P(F("AT+CIPSTATUS"), mdm_start, mdm_empty, 5000) == 0);
	// Brings Up Wireless Connection
	if (sendATcommand4_P(F("AT+CIICR"), mdm_ok, mdm_error, 30000) != 1) return false;
	// Waits for status IP GPRSACT
	while (sendATcommand4_P(F("AT+CIPSTATUS"), mdm_gprsact, mdm_empty, 5000) == 0);
	// Gets Local IP Address
	if (sendATcommand4_P(F("AT+CIFSR"), mdm_ip_ok, mdm_error, 15000) != 1) return false;
	// Waits for status IP STATUS
	while (sendATcommand4_P(F("AT+CIPSTATUS"), mdm_ip_status, mdm_empty, 5000) == 0);
	//delay(5000);
#ifdef _TRACE
	Serial.println(F("Openning TCP/UDP")); //��� �������� ������� ��������� �� ��������(���� ����� ���������������)
#endif
	if (sendATcommand4_P(F(data_ip_protocol), mdm_cnct_ok, mdm_cnct_fail, 30000) != 1) return false;
#ifdef _TRACE
	Serial.println(F("GPRS OK"));
#endif
	return true;
}
boolean VGSM3::SendATCommand() {
#ifdef _TRACE
	Serial.println(F("SendATCommand"));
#endif
	if (sendATcommand4_P(F("AT"), mdm_ok, mdm_error, 2000, 1000) != 1) return false; //���������� � �������� ���� � ����� ������� �������� 
	NewSMS(); //��������� ������ � ������� �� ������� ������� � ������� ����� ���, ���� ������, ������� �� ������ ��� ������ ������

	return true; //���������� ����� � ���, ��� ����� �����������, ����� ���������� ������� � �������
	//???���������, ��� �����, ���� ����� �� �����������. ������ ������ ����� ���� ������ � ��� ��������
}
int VGSM3::TCPSocketResponse() {
	if (strstr(serial_buff, "#" MAC_ADDRESS) == NULL) return 0; //���� ������� ������ �� ������� � ������ ����������� ����� ��������
	int first = 0; 
	int second = 0;
	first = (strstr(serial_buff, "#" MAC_ADDRESS));
	second = strlen("#" MAC_ADDRESS "#T1#");
	char reply[3];
	memset(reply, '\0', sizeof(reply));
	//ClearMemoryBuffer(reply);
	int last = (strstr(serial_buff + first + second, "#"));
	int len = (last - first - 1);
	strncpy(reply, serial_buff + first + second, len);
	tcp_index = 0;
	if (strstr(reply, "ON")) tcp_index = ON;
	if (strstr(reply, "OFF")) tcp_index = OFF;
}
int VGSM3::NewSMS() {
#ifdef _TRACE
	Serial.println(F("NewSMS"));
#endif
	boolean smsflag, tcpflag = false;
	//Serial.println(buffer);
	//char cmd[] = "+CMTI:";// \"ME\","; //������ ������� � ������� ������ ����� ���������
	DeleteSpaceAndUpper();//������� ������� � ��������� �������
#ifdef _TRACE
	Serial.println(serial_buff);
#endif
	//��������� ������� � ������ ������� ������ ���������
	if (strstr_P(serial_buff, tmpl_sms) != NULL) smsflag = true;
	if (strstr_P(serial_buff, tmpl_tcp) == NULL) tcpflag = true;

	if ((!smsflag) && (!tcpflag)) return 0;//��������� ���, �� � ������ ������ ������
	//����� ������� ������ ���������
#ifdef _TRACE
	Serial.println("get new sms");
#endif
	int first = 0;
	if(tcpflag) first = (strstr_P(serial_buff, tmpl_tcp) - serial_buff) + strlen_P(tmpl_tcp) + 2;
	if(smsflag) first = (strstr_P(serial_buff, tmpl_sms) - serial_buff) + strlen_P(tmpl_sms) + 5; //11;// ������� ����� ������ ������ ���������, �� ��� ����� ����� �� �����, ������ ��������� ���������
#ifdef _TRACE
	//Serial.println(strlen(cmd));
	Serial.print(F("ser buf len: "));
	Serial.print(strlen(serial_buff));
	Serial.print(F(" first pos: "));
	Serial.println(first);
#endif
	// ������ \n �������� \r\n ���� ���������
	int last = (strstr(serial_buff + first, "\r\n") != NULL) ? 
		(strstr(serial_buff + first, "\n") - serial_buff) : //������� ����� ������
		strlen(serial_buff);
	Serial.println(last);

	char reply[3];
	memset(reply, '\0', sizeof(reply));
	//ClearMemoryBuffer(reply);
	int sms_ind_len = (last - first - 1);
	strncpy(reply, serial_buff + first, (sms_ind_len <= 2)?sms_ind_len:2); //�������� ����� ����� ��������� �� ����� ������
#ifdef _TRACE
	Serial.println(reply);
#endif
	if (tcpflag) tcp_index = atoi(reply); //��������� ��� � ����� � ������ � ������, ������ ��� ����� ��������� � ��������� �����
	if (smsflag) sms_index = atoi(reply); //��������� ��� � ����� � ������ � ������, ������ ��� ����� ��������� � ��������� �����
}
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
	
	strcpy_P(out_msg_buff, help_msg);
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

	sprintf_P(out_msg_buff, PSTR("%S%d"), gsci_msg, sms_index);
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
	if (sendATcommand4_P(F("AT+CMGF=1"), mdm_ok, mdm_error, 2000, 1000) != 1) return;
#ifdef _TRACE
	Serial.print(">cmgf>");
	Serial.println(serial_buff);
#endif
	memset(aux_str, '\0', sizeof(aux_str));
	sprintf(aux_str, "AT+CMGS=\"%s\"\0", phone);
	sendATcommand4(aux_str, mdm_ok, mdm_error, 2000);
	sendATcommand4(text, mdm_ok, mdm_error, 2000, 3000);
	//GSMport.write(0x1A); // ��������� ������ � ������� �������
						 //GSMport.write(0x0D);
						 //GSMport.write(0x0A);
#ifdef _TRACE
	Serial.print(">cmgs>");
	Serial.println(serial_buff);
#endif
}
void VGSM3::ReadSMS() {
	if (sms_index == 0) return;
	if (sendATcommand4_P(F("AT+CMGF=1"), mdm_ok, mdm_error, 2000, 1000) != 1) return; //��������� ����� � ��������� �����
#ifdef _TRACE
	Serial.println(serial_buff);
#endif
	if (sendATcommand4_P(F("AT+CPMS= \"MT\""), mdm_ok, mdm_error, 2000, 1000) != 1) return; //��������� ����� � ��������� �����
#ifdef _TRACE
	Serial.println(serial_buff);
	Serial.println(sms_index);
	Serial.println("6");
#endif
	memset(aux_str, '\0', sizeof(aux_str));
	sprintf(aux_str, "AT+CMGR=%d\0", sms_index);
	if (sendATcommand4(aux_str, mdm_ok, mdm_error, 2000) != 1) return;
	DeleteSpaceAndUpper(); //������� �� ����������� ��������� ������� � ��������� �������
#ifdef _TRACE
	Serial.println(serial_buff);
#endif
	memset(in_msg_buff, '\0', sizeof(in_msg_buff));
	char cmd[] = "\r\n"; // �������� �� \r\n  ���� ���������
	int first = (strstr(serial_buff, cmd) - serial_buff) + strlen(cmd);//���� ������ ��������� �������� ������
	first = (strstr(serial_buff + first, cmd) - serial_buff) + strlen(cmd);//���� ������ ��������� �������� ������, ����� ���� ����� ����� ���������
	int last = (strstr(serial_buff + first, cmd) != NULL) ?
		(strstr(serial_buff + first, cmd) - serial_buff) : //������� ����� ������
		strlen(serial_buff);


	strncpy(in_msg_buff, serial_buff + first, ((last - first - 1) > 13) ? 13 : (last - first - 1));
#ifdef _TRACE
	Serial.println("7");
	Serial.println(in_msg_buff);
#endif
	memset(aux_str, '\0', sizeof(aux_str));
	sprintf(aux_str, "AT+CMGD=%d,0\0", sms_index);
	if (sendATcommand4(aux_str, mdm_ok, mdm_error, 2000) != 1) return;
#ifdef _TRACE
	Serial.println(serial_buff);
#endif
	sms_index = 0;
}
void VGSM3::ReadTCP() {
	if (tcp_index == 0) return;
	
	memset(aux_str, '\0', sizeof(aux_str));
	sprintf(aux_str, "AT+CIPRXGET=2,%d,%d\0", sms_index, strlen(serial_buff));
	if (sendATcommand4(aux_str, mdm_ok, mdm_error, 2000) != 1) return;
	DeleteSpaceAndUpper(); //������� �� ����������� ��������� ������� � ��������� �������
#ifdef _TRACE
	Serial.println(serial_buff);
#endif
	memset(in_msg_buff, '\0', sizeof(in_msg_buff));
	char cmd[] = "\r\n";
	int first = (strstr(serial_buff, cmd) - serial_buff) + strlen(cmd);//���� ������ ��������� �������� ������
	int last  = (strstr(serial_buff + first, cmd) - serial_buff) + strlen(cmd);//���� ������ ��������� �������� ������, ����� ���� ����� ����� ���������
	
	strncpy(in_msg_buff, serial_buff + first, ((last - first - 1) > 13) ? 13 : (last - first - 1));
#ifdef _TRACE
	Serial.println("7");
	Serial.println(in_msg_buff);
#endif
	tcp_index = 0;
}
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
boolean  VGSM3::send_data2()
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
	memset(buf_ip_data, '\0', sizeof(buf_ip_data));
	memset(aux_str, '\0', sizeof(aux_str));
	//sprintf(buf_ip_data, "#" MAC_ADDRESS "\n #T1# 26.05\n #Z1# 1 \n ##");
	sprintf(buf_ip_data, "#" MAC_ADDRESS "#T1#26.05#");
	sprintf(aux_str, "AT+CIPSEND=%d", strlen(buf_ip_data));  //��������� ������ �����  ���� ������  ����� ������ �  ������  buf_ip_data

	if (sendATcommand4(aux_str, mdm_arrow, mdm_error, 10000) == 1)  //���� �������� ����������� ">"  ������� buf_ip_data � ����. 
	{
		if (sendATcommand4(buf_ip_data, mdm_send_ok, mdm_error, 10000) == 1) //���� �� ������ ���������, �� � ����� ������� "SEND OK"
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
		sendATcommand4_P(F("AT+CIPCLOSE"), mdm_close_ok, mdm_error, 10000);
		//powerpulse();

		Serial.println(F("SEND TO GPRS FAILED")); //��� �������� ������� ��������� �� ��������(���� ����� ���������������)
		return false;
	}
	// Closes the socket
	// sendATcommand2("AT+CIPCLOSE", "CLOSE OK", "ERROR", 10000);
	//digitalWrite(ledPin, LOW);    // ��������� LED
}
