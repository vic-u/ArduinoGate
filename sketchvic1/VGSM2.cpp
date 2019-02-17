#include "VGSM2.h"

VGSM2::VGSM2(){}
VGSM2::~VGSM2(){}
//инициализация скорости порта модема и вывод на лцд дисплей информации про модем
void VGSM2::InitGSM(MYLCD& lcd) {
#ifdef _TRACE
	Serial.println(F("InitGSM"));
#endif
	GSMport.begin(9600); //открываем порт модема на скорости 9600
#ifdef _TRACE
	Serial.println(F("InitGSM"));
#endif
	lcd.gsmInit();
	//buffer = new String();
//	buffer.reserve(256);
	//lastphone.reserve(14);
	//msg.reserve(30);
	ClearBuffer();
}
void VGSM2::ClearMemoryBuffer(char memory_buffer[]) {
	memset(memory_buffer, '\0', sizeof(memory_buffer));
}
///заполняем буффер для чтения ответов из порта модема 0
void VGSM2::ClearBuffer() {
	ClearMemoryBuffer(serial_buff);
}
///заполняем буффер для отправки ответного смс  0
void VGSM2::ClearOutMsgBuffer() {
	ClearMemoryBuffer(out_msg_buff);
}
///заполняем буффер для номера телефона 0
void VGSM2::ClearOutPhnBuffer() {
	ClearMemoryBuffer(out_phn_buff);
}
///заполняем буффер для выделенного шаблона в сообщении 0
void VGSM2::ClearInMsgBuffer() {
	ClearMemoryBuffer(in_msg_buff);
}
///читаем данные из порта модема, после отправки команд. все вычитываем в буфер 300 байт. что выше отправляем в пустышку
void VGSM2::ReadBuffer() {
	//Serial.println("ReadBuffer");
	//Serial.println(sizeof(serial_buff));
	ClearBuffer(); //забиваем буфер нулями
	p_serial_buff = &serial_buff[0]; //переходим к началу буфера
	int count = 0; //счетчик прочитанных из буффера данных, чтобы не уйти за границу массива
	char empty = 0x00; //вводим переменную, в которую будем писать, если буфер закончится, а порт еще будет слать данные

	while (GSMport.available()) {  //пока не кончатся данные в порте читаем их в строку 
		
		count++; // увеличиваем счетчик длины буфер, чтобы не проспать его окончание
		if (count < sizeof(serial_buff)) { //пока мы внутри буфера, читаем в него
			*p_serial_buff = GSMport.read(); //читаем очередной символ в строку
			p_serial_buff++; //сдвигаем буфер на следующий символ
			//Serial.println("av");
		}
		else {
			empty = GSMport.read(); //читаем данные в пустую переменную, надо же их дочитать
		}
		delay(10);
		//p_serial_buff++;
	}
	*p_serial_buff = 0x00; //заполняем последний символ нулем, чтобы получить с строку
}
/// удаляем пробелы и поднимаем регистр
void VGSM2::DeleteSpaceAndUpper()
{
	int i = 0, j = 0;

	while (serial_buff[i] != '\0') //идем по заполненному буферу пока не встретим конец
	{
		if (serial_buff[i] != ' ') //если текущий символ в буфере не пробел
		{
			/*serial_buff[j] = serial_buff[i];
			serial_buff[j] = toupper(serial_buff[j]);*/
			serial_buff[j] = toupper(serial_buff[i]);
			++j;
		}

		++i;
	}

	serial_buff[j] = '\0';
}
//перезагружаем модем при запуске ардуины, так как при перезапуске ардуины модем 850 аппаратно не перегружается,  делаем это програмно отдельно
void VGSM2::Reset() {
#ifdef _TRACE
	Serial.println(F("Send Reset"));
#endif	
	GSMport.println(F("AT+CFUN=1,1"));  //команда перезагрузки модема отправляем в порт модема
	delay(10000); // ждем 10 секунд
	ReadBuffer(); //смотрим ответ модема и выводим в консоль
#ifdef _TRACE
	Serial.println(serial_buff);
	Serial.println("1");
#endif
	GSMport.println(F("AT"));  //первая команда в модем, для проверки, что оклемался после перезагрузки
	delay(10000); // ждем 10 секунд чтобы все строки модем выдал в буфер
	ReadBuffer(); //смотрим ответ модема и выводим в консоль
#ifdef _TRACE
	Serial.println(serial_buff);
	Serial.println("1.1");
#endif
	GSMport.println(F("AT+CPMS= \"MT\""));  // переключаем хранилище смс на сим карту и телефон
	delay(10000);
	ReadBuffer(); //смотрим ответ модема и выводим в консоль
#ifdef _TRACE
	Serial.println(serial_buff);
	Serial.println("3");
#endif

	for (int i = 1; i < 11; i++) { // удаляем сообщения 
		GSMport.print(F("AT+CMGD="));
		GSMport.print(i);
		GSMport.println(F(",0"));
		delay(1000);
		ReadBuffer(); //смотрим ответ модема и выводим в консоль
#ifdef _TRACE
		Serial.println(serial_buff);
#endif
	}

}
/**
проверяем в буфере, отправленном командой АТ, появление данных о приходе новой смс, которую надо обработать
*/
int VGSM2::NewSMS() {
#ifdef _TRACE
	Serial.println(F("NewSMS"));
#endif
	//Serial.println(buffer);
	char cmd[] = "+CMTI:";// \"ME\","; //шаблон наличия в буффере данных новых сообщений
	DeleteSpaceAndUpper();//удаляем пробелы и поднимаем регистр
#ifdef _TRACE
	Serial.println(serial_buff);
#endif
	//проверяем наличие в буфере шаблона нового сообщения
	if (strstr(serial_buff, cmd) == NULL)  return 0; //сообщения нет, ну и нечего больше делать
	//нашли признак нового сообщения
#ifdef _TRACE
	Serial.println("get new sms");
#endif
	int first = (strstr(serial_buff, cmd) - serial_buff) + strlen(cmd) + 5; //11;// находим конец шалона нового сообщения, за ним будем номер на симке, откуда прочитать сообщение
#ifdef _TRACE
	//Serial.println(strlen(cmd));
	Serial.print(F("ser buf len: "));
	Serial.print(strlen(serial_buff));
	Serial.print(F(" first pos: "));
	Serial.println(first);
#endif
	int last = (strstr(serial_buff + first, "\n") != NULL) ?
		(strstr(serial_buff + first, "\n") - serial_buff) : //находим конец строки
		strlen(serial_buff);
	Serial.println(last);

	char reply[3];
	ClearMemoryBuffer(reply);
	strncpy(reply, serial_buff + first, last - first - 1); //забираем номер номер сообщения из общей строки
#ifdef _TRACE
	Serial.println(reply);
#endif
	return atoi(reply); //переводим его в цифру и кладем в индекс, откуда его потом прочитаем в очередном цикле
}
//отправляем в модем команду АТ для проверки жив или нет, заодно читаем остатки входного буфера, вдруг там пришло новое сообщение
boolean VGSM2::SendATCommand() {
#ifdef _TRACE
	Serial.println(F("Send AT"));
#endif
	GSMport.println(F("AT"));  //отправляем в софтваре порт в модем команду проверки 
	delay(1000);
	ReadBuffer(); // читаем ответ в общий буфер serial_buff, туда попадет все, что накопилось
	index = NewSMS(); //разбираем данные в буффере на наличие команды о приходе новой смс, если пришла, заберем ее индекс для чтения данных
	//Serial.println(index);
	return (strstr(serial_buff, "OK") != NULL) ? true : false; //возвращаем ответ о том, что модем откликнулся, иначе перегрузим ардуино с модемом
	//???проверить, что будет, если модем не откликнется. буффер должен тогда быть пустым и что случится
}
void VGSM2::ReadSMS() {
	if (index == 0) return;
	GSMport.println(F("AT+CMGF=1")); //переводим модем в текстовый режим
	delay(1000); // ждем 1 секунд
	ReadBuffer(); //смотрим ответ модема и выводим в консоль
#ifdef _TRACE
	Serial.println(serial_buff);
#endif
	GSMport.println(F("AT+CPMS= \"MT\""));  // переключаем хранилище смс на сим карту  и память телефона
	delay(1000);
	ReadBuffer(); //смотрим ответ модема и выводим в консоль
#ifdef _TRACE
	Serial.println(serial_buff);
	Serial.println(index);
	Serial.println("6");
#endif
	GSMport.print(F("AT+CMGR=")); // формируем команду на прочтение из модема сообщения под номером index
	GSMport.println(index);
	delay(1000);
	ReadBuffer();
	DeleteSpaceAndUpper(); //убираем из прочтенного сообщения пробелы и поднимаем регистр
#ifdef _TRACE
	Serial.println(serial_buff);
#endif
	ClearInMsgBuffer();
	if (strstr(serial_buff, "OK") != NULL) {
		Serial.println("OK");
		char cmd[] = "\n";
		int first = (strstr(serial_buff, cmd) - serial_buff) + strlen(cmd);//ищем первое вхождение перевода строки
		first = (strstr(serial_buff + first, cmd) - serial_buff) + strlen(cmd);//ищем второе вхождение перевода строки, после него будет текст сообщения
		int last = (strstr(serial_buff + first, cmd) != NULL) ?
			(strstr(serial_buff + first, cmd) - serial_buff) : //находим конец строки
			strlen(serial_buff);
		
		
		strncpy(in_msg_buff , serial_buff + first, ((last - first - 1) > 13) ? 13 : (last - first - 1));
		Serial.println("7");
		Serial.println(in_msg_buff);
		GSMport.print(F("AT+CMGD="));  ////выполняем команду удаления сообщения из памяти модема
		GSMport.print(index);  ////выполняем команду удаления сообщения из памяти модема
		GSMport.println(F(",0"));  //отправляем в модем
		delay(1000);
		ReadBuffer();
		Serial.println(serial_buff);
		index = 0;
	}
}
boolean VGSM2::CheckSMSCommand(Heater& htr, Water& wtr)
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
	if (strstr_P(in_msg_buff,cmd_start_no_heat) != NULL) //start no heat
	{
		wtr.setCommand(RC_DEVICEON);
		return true;
	}
	//stop all
	if (strstr_P(in_msg_buff,cmd_stop_all) != NULL) // stop all
	{
		htr.setCommand(RC_DEVICEOFF);
		wtr.setCommand(RC_DEVICEOFF);
		return true;
	}
	// stop no heat
	if (strstr_P(in_msg_buff,cmd_stop_no_heat) != NULL)
	{
		wtr.setCommand(RC_DEVICEOFF);
		return true;
	}
	// stop only heat
	if (strstr_P(in_msg_buff,cmd_stop_only_heat) != NULL)
	{
		htr.setCommand(RC_DEVICEOFF);
		return true;
	}
	//status
	if (strstr_P(in_msg_buff,cmd_status) != NULL)
	{
#ifdef _TRACE
		Serial.println(F("sms status"));
#endif
		return true;
	}
	//set temp
	if (strstr_P(in_msg_buff,cmd_set_temp) != NULL)
	{
		int t = 0;
		if (ConvertTempChr(in_msg_buff, t))
		{
			htr.setMaxRoomTemp(t);
			ClearOutMsgBuffer();
			sprintf_P(out_msg_buff, PSTR("%S%d"), ts_msg, t);
			SendSMSChr(out_msg_buff, out_phn_buff);
		}
#ifdef _TRACE
		Serial.println(F("set temp"));
#endif
		return true;
	}
	//set delta
	if (strstr_P(in_msg_buff,cmd_set_delta) != NULL)
	{
		int t = 0;
		if (ConvertTempChr(in_msg_buff, t))
		{
			htr.setDeltaRoomTemp(t);
			ClearOutMsgBuffer();
			sprintf_P(out_msg_buff, PSTR("%S%d"), ds_msg, t);
			SendSMSChr(out_msg_buff, out_phn_buff);
		}
#ifdef _TRACE
		Serial.println(F("set delta temp"));
#endif
		return true;
	}
	//set help
	if (strstr_P(in_msg_buff,cmd_help) != NULL)
	{
		SendInitSMSChr();
#ifdef _TRACE
		Serial.println(F("sms help"));
#endif
		return true;
	}
	return false; //unknown command
}
void VGSM2::SendInitSMSChr()
{
#ifdef _TRACE
	Serial.println(F("Init SMS Send"));
#endif	
	ClearOutMsgBuffer();
	ClearOutPhnBuffer();
	
	strcpy_P(out_msg_buff, help_msg);
	strcpy(out_phn_buff, PHONENUM);
	Serial.println("5");
	Serial.println(out_msg_buff);
	Serial.println(out_phn_buff);

	
	SendSMSChr(out_msg_buff, out_phn_buff);
	
	delay(2000);
}
void VGSM2::SendSMSChr(char text[], char phone[]) {
	Serial.println(F("Send sms char"));
	GSMport.println(F("AT+CMGF=1")); //переводим модем в текстовый режим
	delay(1000);
	ReadBuffer(); //смотрим ответ модема и выводим в консоль
	Serial.print(">cmgf>");
	Serial.println(serial_buff);

	GSMport.print(F("AT+CMGS=\""));//отправляем команду на отправку сообщения
	GSMport.print(phone); //на номер
	GSMport.println(F("\""));
	delay(1000);
	GSMport.print(text); //с текстом
	delay(1000);
	GSMport.write(0x1A); // окончание строки и перевод каретки
						 //GSMport.write(0x0D);
						 //GSMport.write(0x0A);
	delay(3000);
	ReadBuffer(); //смотрим ответ модема и выводим в консоль
	Serial.print(">cmgs>");
	Serial.println(serial_buff);
}
void VGSM2::StatusChr(double roomtemp, boolean wtrflag, boolean htrflag, int free_ram)
{
	char str_rt[6];
	Serial.println("StatusChr");
	dtostrf(roomtemp, 4, 2, str_rt);
	str_rt[5] = 0x00;
	Serial.println(str_rt);
	ClearOutMsgBuffer();

	sprintf_P(out_msg_buff, PSTR("%S%s%S%S%S%S%S%S%S%S%d%s"),
		rt_msg, str_rt, sn_msg, 
		water_msg, wtrflag ? on_msg : off_msg, sn_msg,
		htr_msg, htrflag ? on_msg : off_msg, sn_msg,
		fr_msg, free_ram, "\0");
	//sprintf_P(out_msg_buff, PSTR("%S%s"),
	//	rt_msg, str_rt); 
#ifdef _TRACE
	Serial.println(out_msg_buff);
#endif
	SendSMSChr(out_msg_buff, out_phn_buff);
	delay(2000);
}
boolean VGSM2::ConvertTempChr(char * command, int &t)
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

