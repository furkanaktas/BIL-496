#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>

#include"inc/hw_memmap.h" // periperhals base adresses
#include"inc/hw_ints.h" // interrupts base adresses

#include"driverlib/sysctl.h"
#include"driverlib/gpio.h"  // gpio periperhals,  uart gpio üzerinden çalýþýr
#include"driverlib/uart.h" // uart için


#include"driverlib/adc.h" // adc periperhals

#include "driverlib/timer.h"


#include <string.h>

#include "inc/hw_types.h"
#include "driverlib/fpu.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"  // uart pinlerinin yerlerini göstermek için

//#include "driverlib/pwm.h"

#include "driverlib/adc.h"

#include <math.h>
#include "inc/hw_timer.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "inc/hw_nvic.h"


#include "utils/uartstdio.h"
#include "utils/cmdline.h"


#include "LCD.h"

#include "inc/TM4C123GH6PM.h"



static char ReceivedData[512];
static char localDB[512][36];
static unsigned long milliseconds = 0;
static int localIdx =0;
static int runMode =0;  // 0 local , 1 online
static int curr =0;



void delayMs(uint32_t ui32Ms) {

    // 1 clock cycle = 1 / SysCtlClockGet() second
    // 1 SysCtlDelay = 3 clock cycle = 3 / SysCtlClockGet() second
    // 1 second = SysCtlClockGet() / 3
    // 0.001 second = 1 ms = SysCtlClockGet() / 3 / 1000

    SysCtlDelay(ui32Ms * (SysCtlClockGet() / 3 / 1000));
}

void delayUs(uint32_t ui32Us) {
    SysCtlDelay(ui32Us * (SysCtlClockGet() / 3 / 1000000));
}




void Timer5IntHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    milliseconds++;
}

void timerInit(void)
{

    SysTickPeriodSet(0x00FFFFFF);
    SysTickEnable();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC_UP);

    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()/1000);


    TimerIntRegister(TIMER0_BASE, TIMER_A, Timer5IntHandler);
    IntEnable(INT_TIMER0A);

    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);

    IntMasterEnable();

}


unsigned long millis(void)
{
    return milliseconds;
}


void InitUART(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);
}

void InitESPUART(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);
    UARTConfigSetExpClk(UART1_BASE, 16000000, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    UARTEnable(UART1_BASE);

}

int SearchIndexOf(char src[], char str[])
{
   int i, j, firstOcc;
   i = 0, j = 0;

   while (src[i] != '\0')
   {

      while (src[i] != str[0] && src[i] != '\0')
         i++;

      if (src[i] == '\0')
         return (-1);

      firstOcc = i;

      while (src[i] == str[j] && src[i] != '\0' && str[j] != '\0')
      {
         i++;
         j++;
      }

      if (str[j] == '\0')
         return (firstOcc);
      if (src[i] == '\0')
         return (-1);

      i = firstOcc + 1;
      j = 0;
   }

   return (-1);
}

void flushUART(){

    while(UARTCharsAvail(UART1_BASE))
    {
        UARTCharGet(UART1_BASE);
        UARTprintf("uart flushing...\n");
    }
    memset(ReceivedData, 0, sizeof(ReceivedData));
}

void SendATCommand(char *cmd)
{
    while(UARTBusy(UART1_BASE));
    while(*cmd != '\0')
    {
        UARTCharPut(UART1_BASE, *cmd++);
    }
    UARTCharPut(UART1_BASE, '\r'); //CR
    UARTCharPut(UART1_BASE, '\n'); //LF

}

int recvString(char *target, char *data, int timeout, bool check)
{
    int i=0;
    char a;
    unsigned long start = millis();
    while (millis() - start < timeout)
    {
        while(UARTCharsAvail(UART1_BASE))
        {
              a = UARTCharGet(UART1_BASE);
              if(a == '\0') continue;
              data[i]= a;
              i++;
        }

        if(check)
        {
            if (SearchIndexOf(data, target) != -1)
            {
                break;
            }
        }
    }

    return 0;
}

bool recvFind(char *target, int timeout,bool check)
{
    recvString(target, ReceivedData, timeout, check);

    //UARTprintf("Received Data: \n%s\n", ReceivedData);


    if (SearchIndexOf(ReceivedData, target) != -1)
    {
        return true;
    }
    return false;
}


bool ATesp(void)
{
    memset(ReceivedData, 0, sizeof(ReceivedData));
    SendATCommand("AT");
    return recvFind("OK",5000, true);
}

bool RSTesp(void)
{
    memset(ReceivedData, 0, sizeof(ReceivedData));
    SendATCommand("AT+RST");
    return recvFind("OK",5000, false);
}

bool CWMODEesp(int mode)
{
    memset(ReceivedData, 0, sizeof(ReceivedData));
    if(mode == 1)
        SendATCommand("AT+CWMODE=1");  //client
    else if(mode == 2)
        SendATCommand("AT+CWMODE=2");  // host
    else
        SendATCommand("AT+CWMODE=3");  // both

    return recvFind("OK",5000, true);
}

bool CWJAPesp(void)
{
    memset(ReceivedData, 0, sizeof(ReceivedData));
    SendATCommand("AT+CWJAP=\"Aktas\",\"GireBiLirseN61\""); //Your Wifi: NetworkName, Password
    return recvFind("OK",10000, true);
}

bool CWQAPesp(void)
{
    memset(ReceivedData, 0, sizeof(ReceivedData));  // quit from AP
    SendATCommand("AT+CWQAP");
    return recvFind("OK",10000, true);
}

bool CWSAPesp(void)
{
    memset(ReceivedData, 0, sizeof(ReceivedData));
    SendATCommand("AT+CWSAP=\"ESP12E\",\"6101461ts\",4,3"); // SSID, password, channel, encrpt method
    return recvFind("OK",10000, true);
}

bool CIPAPesp(){
    memset(ReceivedData, 0, sizeof(ReceivedData));
    SendATCommand("AT+CIPAP=\"192.168.61.61\"");
    return recvFind("OK",5000, true);
}

bool CIPMUXesp(void)
{
    memset(ReceivedData, 0, sizeof(ReceivedData));
    SendATCommand("AT+CIPMUX=1");
    return recvFind("OK",5000, true);
}

bool CIPSERVEResp(int mode)
{
    memset(ReceivedData, 0, sizeof(ReceivedData));
    if(mode == 0)
        SendATCommand("AT+CIPSERVER=0");  //close server
    else if(mode == 1)
        SendATCommand("AT+CIPSERVER=1,8080");  //create server on port 8080

    return recvFind("OK",10000, true);
}

bool CWLIFesp()
{
    memset(ReceivedData, 0, sizeof(ReceivedData));
    SendATCommand("AT+CWLIF");  // list of connected clients
    return recvFind("OK",5000, true);
}

bool CIPSTARTesp(void)
{
    memset(ReceivedData, 0, sizeof(ReceivedData));
    SendATCommand("AT+CIPSTART=\"TCP\",\"23.97.238.10\",5000"); //Server IP and Port: such as 192.255.0.100, 9999
    return recvFind("OK",2000, true);
}

bool CIPCLOSEesp(int connectionID, bool isServer)
{
    memset(ReceivedData, 0, sizeof(ReceivedData));

    if(isServer){
        //char cmd[13];
        //sprintf(cmd, "AT+CIPCLOSE=%d", connectionID);

        if(connectionID ==0)
            SendATCommand("AT+CIPCLOSE=0");
        else if(connectionID ==1)
            SendATCommand("AT+CIPCLOSE=1");
        else if(connectionID ==2)
            SendATCommand("AT+CIPCLOSE=2");
        else
            SendATCommand("AT+CIPCLOSE=3");

        //SendATCommand(cmd);
    }else {
        SendATCommand("AT+CIPCLOSE");
    }
    return recvFind("OK",5000, true);
}


bool CIPSENDesp(char data[36], int connectionID, bool isServer)
{
    memset(ReceivedData, 0, sizeof(ReceivedData));
    //UARTprintf("deneme 1");
/*
    uint32_t intStat;
    intStat = UARTIntStatus(UART0_BASE, true);
    UARTIntClear(UART0_BASE, intStat);
*/
    if(isServer){

        if(connectionID ==0)
            SendATCommand("AT+CIPSEND=0,3");
        else if(connectionID ==1)
            SendATCommand("AT+CIPSEND=1,3");
        else if(connectionID ==2)
            SendATCommand("AT+CIPSEND=2,3");
        else
            SendATCommand("AT+CIPSEND=3,3");

       // SendATCommand(command);
        //recvFind("OK",2000, true);
        delayMs(5);
        //memset(ReceivedData, 0, sizeof(ReceivedData));
        if(runMode == 1){
            SendATCommand("1");        // send OK to client
        }
        else{
            SendATCommand("0");        // send OK to client
        }
    }else{
        UARTprintf("post req... \n");

        char first[8] = "POST /?";
        char last[29] = " HTTP/1.1\r\nHost: 0.0.0.0\r\n\r\n";
        char postData[71];
        memset(postData, '\0', sizeof(postData));

        strcpy(postData, first);
        strcat(postData, data);
        strcat(postData, last);

        /*
        if(postData[70] == '\0')
            UARTprintf("son : \\ 0  \n");

        if(postData[70] == '\n')
            UARTprintf("son : \\ n  \n");

        int len = strlen(postData);
        UARTprintf("::::: %s  , %d , %d",postData, len, strlen("POST /?ghID=0&temp=21.75&hum=32.63&level=3 HTTP/1.1\r\nHost: 0.0.0.0\r\n\r\n"));
*/


        char length[4];
        memset(length, '\0', sizeof(length));
        sprintf(length, "%d", strlen(postData));

        char cmd[15] = "AT+CIPSEND=";
        strcat(cmd, length);

        UARTprintf("cmd  : :  %s \n", cmd);

        //SendATCommand("AT+CIPSEND=70");
        SendATCommand(cmd);

        delayMs(500);
        flushUART();

        //SendATCommand("POST /?ghID=0&temp=21.75&hum=32.63&level=3 HTTP/1.1\r\nHost: 0.0.0.0\r\n\r\n"); // sonunda amk
        SendATCommand(postData);

        delayMs(500);
    }
    //UARTprintf("return öncesi");
    return recvFind("SEND OK",2000, true);
}



void createServer(){

    while(true)
    {
        UARTprintf("reseting esp...\n");
        bool TaskStatus = RSTesp();  // reset ESP
        if(TaskStatus)
        {
            UARTprintf("ESP reset!\n");
            break;
        }
    }

    while(true)
    {
        UARTprintf("mux mode = 1\n");
        bool TaskStatus = CIPMUXesp();   // allows multiple connection  max 4
        if(TaskStatus)
        {
            //UARTprintf("Connection is established!\n");
            break;
        }
    }

    while(true)
    {
        UARTprintf("creating server ...\n");
        bool TaskStatus = CIPSERVEResp(1);   // configure as server
        if(TaskStatus)
        {
            UARTprintf("Server created!\n");
            break;
        }
    }
    flushUART();
}


void setOnlineMode(){

    while(true)
    {
        UARTprintf("reseting esp...\n");
        bool TaskStatus = RSTesp();  // reset ESP
        if(TaskStatus)
        {
            UARTprintf("ESP reset!\n");
            break;
        }
    }


    while(true)
    {
        //UARTprintf("Trying to connect wi-fi\n");
        bool TaskStatus = CWMODEesp(3);  // set mode both AP, client
        if(TaskStatus)
        {
            //UARTprintf("Connection is established!\n");
            break;
        }
    }


    //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3, GPIO_PIN_2);
    while(true)
    {
        //UARTprintf("Trying to connect wi-fi\n");
        bool TaskStatus = CWJAPesp();   // connect wifi
        if(TaskStatus)
        {
            //UARTprintf("Connection is established!\n");
            break;
        }
    }

    while(true)
    {
       //UARTprintf("Trying to connect wi-fi\n");
       bool TaskStatus = CWSAPesp();   // set AP  ssid pass
       if(TaskStatus)
       {
           //UARTprintf("Connection is established!\n");
           break;
       }
    }

    while(true)
    {
        //UARTprintf("Trying to connect wi-fi\n");
        bool TaskStatus = CIPAPesp();  // set AP ip to static IP
        if(TaskStatus)
        {
            //UARTprintf("Connection is established!\n");
            break;
        }
    }
/*
    while(true)
    {
        UARTprintf("reseting esp...\n");
        bool TaskStatus = RSTesp();  // reset ESP
        if(TaskStatus)
        {
            UARTprintf("ESP reset!\n");
            break;
        }
    }
*/
    flushUART();
    createServer();
}

void setLocalMode(){
    while(true)
    {
        UARTprintf("reseting esp...\n");
        bool TaskStatus = RSTesp();  // reset ESP
        if(TaskStatus)
        {
            UARTprintf("ESP reset!\n");
            break;
        }
    }

    while(true)
    {
        //UARTprintf("Trying to connect wi-fi\n");
        bool TaskStatus = CWMODEesp(2);  // set mode AP
        if(TaskStatus)
        {
            //UARTprintf("Connection is established!\n");
            break;
        }
    }

    while(true)
    {
        //UARTprintf("Trying to connect wi-fi\n");
        bool TaskStatus = CWSAPesp(); // set AP  ssid pass
        if(TaskStatus)
        {
            //UARTprintf("Connection is established!\n");
            break;
        }
    }

    while(true)
    {
        //UARTprintf("Trying to connect wi-fi\n");
        bool TaskStatus = CIPAPesp();  // set AP ip to static IP
        if(TaskStatus)
        {
            //UARTprintf("Connection is established!\n");
            break;
        }
    }
/*
    while(true)
    {
        UARTprintf("reseting esp...\n");
        bool TaskStatus = RSTesp();  // reset ESP
        if(TaskStatus)
        {
            UARTprintf("ESP reset!\n");
            break;
        }
    }
*/
    flushUART();
    createServer();
}




void closeServerByResetting(){
/*
    while(true)
    {
        UARTprintf("closing server ...\n");
        bool TaskStatus = CIPSERVEResp(0);   // close server
        if(TaskStatus)
        {
            //UARTprintf("Connection is established!\n");
            break;
        }
    }
*/
    while(true)
    {
        UARTprintf("reset esp\n");
        bool TaskStatus = RSTesp();   // reset ESP
        if(TaskStatus)
        {
            //UARTprintf("Connection is established!\n");
            break;
        }
    }
    flushUART();
}


bool getData(char result[45]){
    memset(ReceivedData, 0, sizeof(ReceivedData));
    SendATCommand("AT+CIPSTATUS");

    if(recvFind("OK", 5000, false)){
        int index = SearchIndexOf(ReceivedData, "+CIPSTATUS:");

        if(index != -1){
            do{
                curr = curr%3;
                switch(curr){
                    case 0:
                        index = SearchIndexOf(ReceivedData, "+CIPSTATUS:0");
                        ++curr;
                        break;
                    case 1:
                        index = SearchIndexOf(ReceivedData, "+CIPSTATUS:1");
                        ++curr;
                        break;
                    case 2:
                        index = SearchIndexOf(ReceivedData, "+CIPSTATUS:2");
                        ++curr;
                        break;
                }
            }while(index == -1);
        }else{
            curr=0;
        }

        if(index != -1){
            index +=11; // after "+CIPSTATUS:"
            char id = ReceivedData[index];
            UARTprintf("Connection ID: %c\n", id);

            int connectionID = atoi(&id);
            UARTprintf("Connection ID(int): %d\n", connectionID);

            //memset(ReceivedData, 0, sizeof(ReceivedData));   //Clear

            flushUART();

            if(!CIPSENDesp("", connectionID, true))
            {
                UARTprintf("getData false döndü!! \n");
                return false;
            }

            //memset(ReceivedData, 0, sizeof(ReceivedData));

            unsigned long start, timeout;
            int i=0;
            char a;

            timeout = millis();
            while(millis() - timeout < 10000){
            //while(true){
                if(UARTCharsAvail(UART1_BASE))
                {
                    if (SearchIndexOf(ReceivedData, "+IPD,") != -1)
                    {
                        i=0;
                        memset(ReceivedData, 0, sizeof(ReceivedData));

                        start = millis();
                        int idx =-1;
                        while (millis() - start < 5000)
                        {
                            while(UARTCharsAvail(UART1_BASE))
                            {
                                a = UARTCharGet(UART1_BASE);
                                if(a == '\0') continue;
                                //ReceivedData[i]= a;
                                result[i] =a;
                                i++;
                            }

                            idx =SearchIndexOf(result, "\r");
                            if (idx != -1)
                            {
                                break;
                            }
                        }


                        i=0;
                        UARTprintf("str :: %s \n", result);
                        CIPCLOSEesp(connectionID, true);
                        flushUART();

                        if(idx == -1){
                            return false;
                        }

                        for(i = idx; i<45; ++i){
                            result[i] = '\0';
                        }
                       // memset(ReceivedData, 0, sizeof(ReceivedData));
                        //delayMs(100);
                       /*
                        for(i; i<45; ++i){
                            result[i] = '\0';
                        }
                        */


                        return true;
                    }
                    else
                    {
                        a = UARTCharGet(UART1_BASE);
                        if(a == '\0') continue;
                        ReceivedData[i]= a;
                        i++;
                    }
                }
                // timer ekle sonsuz olmasýn ,
            }
            CIPCLOSEesp(connectionID, true);


        }
    }
    UARTprintf("dipteki false !! \n");
    flushUART();
    return false;
}


/*
bool isServerMode(){

    int i =0;
    do{
        memset(ReceivedData, 0, sizeof(ReceivedData));
        SendATCommand("AT+CIPMUX?");
        UARTprintf("while da bekliyor \n");
        if(i >= 5){
            // esp bekleniyor ekrana yaz
        }
        ++i;
    }while(!recvFind("OK", 5000, false));


    int index = SearchIndexOf(ReceivedData, "+CIPMUX:");
    if(index != -1){

        if(ReceivedData[index+8] == '1')
        {
            flushUART();
            return true;
        }
    }

    flushUART();
    return false;

}

bool isThereConnectedClient(){

    int i =0;
    do{
        memset(ReceivedData, 0, sizeof(ReceivedData));
        SendATCommand("AT+CIPSTATUS");
        UARTprintf("while da bekliyor2 \n");
        if(i >= 5){
            // esp bekleniyor ekrana yaz
        }
        ++i;
    }while(!recvFind("OK", 5000, false));

    int index = SearchIndexOf(ReceivedData, "STATUS:");
    if(index != -1){

        if(ReceivedData[index+7] == '3')
        {
            flushUART();
            return true;
        }
    }

    flushUART();
    return false;
}
*/
bool isESPworks(){

    bool running = false;
    unsigned long start = millis();
    while (millis() - start < 30000)    // 30s yanýt bekle
    {

        if (ATesp())
        {
            running = true;
            break;
        }
    }
    /*
    while(!ATesp());

    if(!running){
        //UARTprintf("restart tiva ...");
    }
    */
    flushUART();
    return running;
}


bool connectToDB(){
    unsigned long start = millis();
    bool isOK= false;
    while (millis() - start < 5000)
    {

        if (CIPSTARTesp())
        {
            UARTprintf("connected to server (DB)\n");
            isOK = true;
            break;
        }
    }
    flushUART();
    return isOK;
}

bool dcFromDB(){
    return CIPCLOSEesp(0, false);
}

void postToDB(char post[36]){

    CIPSENDesp(post, 0 , false);
    flushUART();
}

void restartTIVA(){
    HWREG(NVIC_APINT) = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
}


void setSystem(){
    if(runMode == 0){
        setLocalMode();
    }else{
        setOnlineMode();
    }
}


/*
 *  verileri localdeki deðiþkende tut.
 *  gelen baðlantýdan mode deðiþtirme
 *  baðlanýp verileri tel e   alma
 *  mode deðiþtiðinde duruma göre post larý yap, server'a , telefona vs.
 *
 *
 * */

int main(void)
{


    //SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    //SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    timerInit();
    InitUART();
    InitESPUART();
    //Lcd_init();
    UARTprintf("Execute!\r\n");


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // ledler ve switch tuþlarý F portunda

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);  // 1 2 3 pinler , kýrmýzý blue green
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

    delayMs(1000);


    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3, GPIO_PIN_2);

    //setConnections();

    delayMs(1000);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3, 0);

    //ATesp();
    //createServer();
    setSystem(); // start esp in local mode defaultly

    //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3, 0);
    //delayMs(10000);

    char posts[3][36];
    char str[45];

    memset(posts[0], '\0', sizeof(posts[0]));
    memset(posts[1], '\0', sizeof(posts[1]));
    memset(posts[2], '\0', sizeof(posts[2]));
    memset(str, '\0', sizeof(str));

    /*
    posts[0][0] = 't';
    posts[1][0] = 't';
    posts[2][0] = 't';
    */
    bool postTime = false;
    int idx=0, count=0;
    unsigned long stuckTime = millis();
    while(1){

        //UARTprintf("%ul  \n",stuckTime);


        if(!isESPworks()){ // ESP den 30s  cevap alýnamadýysa - ESP yi restart
            setSystem();
            UARTprintf("esp sýkýþtýý .... \n");
            milliseconds =0;
            stuckTime = millis();
        }

        if(millis()-stuckTime >= 900000){   // 15dk (900k ms) iþlem yapýlamadýysa restart
            if(millis()-stuckTime >= 900000){
                UARTprintf("fark 900k \n");
            }
            restartTIVA();
        }
        //memset(str, 0, sizeof(str));

        if(getData(str))
        {


            if(str[5] == 'g' && str[8] == 'D' && strlen(str) >= 33){ // check received msg length
                UARTprintf("idx while : %c  ", str[10]);
                idx= atoi(&str[10]);
                UARTprintf("idx : %d \n",idx);

                int i =5;  // because of "+IPD," start after 5
                if(runMode == 1){   // 1, online mode
                    while(str[i] != '\0'){
                        posts[idx][i-5] = (char)str[i];
                        //UARTprintf("%c", str[i]);
                        ++i;
                    }
                    /*
                    for(i; i-5<36; ++i){
                        posts[idx][i] = '\0';
                    }
                    */

                    if(count >=7)      // 8 defa veri alýnmýþ ama hala tüm serlardan veri gelmediyse
                    {
                        postTime = true;
                    }
                    else
                    {
                        ++count;
                    }
                }
                else{ // local mode
                    memset(localDB[localIdx], '\0', sizeof(localDB[localIdx]));
                    while(str[i] != '\0'){
                        localDB[localIdx][i-5] = (char)str[i];
                        //UARTprintf("%c", str[i]);
                        ++i;
                    }
                    /*
                    for(i; i-5<36; ++i){
                        localDB[localIdx][i] = '\0';
                    }
                    */
                    ++localIdx;

                    for(i=0; i<localIdx; ++i){
                        UARTprintf("received msg ::: %s \n" , localDB[i]);
                    }

                }

            }



            UARTprintf("\n\n");

            memset(str, '\0', sizeof(str));
            milliseconds =0;
            stuckTime = millis();

        }

        if(runMode == 1){ // 1, online mode

            UARTprintf("0: %c   1: %c   2: %c   count: %d \n", posts[0][0], posts[1][0], posts[2][0], count);

            if(posts[0][5] == '0' && posts[1][5] == '1' && posts[2][5] == '2') // tüm seralardan veri geldiyse
            {
                postTime=true;
            }

            if(postTime){
            //if(posts[1][5] == '1' || posts[2][5] == '2'){

                closeServerByResetting();

                // post to DB
                UARTprintf("post to DB \n");

                if(connectToDB()){
                //if(1){
                    int i;
                    for(i=0; i<3;++i){
                        if(posts[i][0] == 'g' && posts[i][3] == 'D' && strlen(posts[i]) >= 33){
                            postToDB(posts[i]);
                        }
                    }

                    dcFromDB();
                }




                delayMs(1000);
                createServer();

                //flushUART();


                memset(posts[0], '\0', sizeof(posts[0]));
                memset(posts[1], '\0', sizeof(posts[1]));
                memset(posts[2], '\0', sizeof(posts[2]));

                /*
                  posts[0][0] = 't';
                  posts[1][0] = 't';
                  posts[2][0] = 't';
                */
                milliseconds =0;
                stuckTime = millis();
                count = 0;
                postTime = false;

            }
        }

        if(true){
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3, GPIO_PIN_3);
            //printf("AT OK !!");
        }
        else{
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3, GPIO_PIN_1);
           // printf("AT Failed !!");
        }

        delayMs(1000);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3, 0);
        delayMs(1000);
/*
        Lcd_Goto(1,1);
        Lcd_Puts("Furkan");
        Lcd_Goto(2,5);
        Lcd_Puts("AKTAS ");
*/
    }



}

