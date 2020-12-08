//*************Khai bao thu vien***************
#include <main.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <Math.h>
#include "Analog.c"
#include "DHT22.c"
#include "ADS1248.c"

//----------------Broker--------------------
//#define BROKER_ADDR            "broker.mqttdashboard.com" // Broker Address
#define BROKER_ADDR              "mqtt.jwclab.com"
#define BROKER_PORT              "1883"                     // Broker Port  
#define BROKER_TOPIC             "jwclab/demo_station"       // Broker Tocpic eqm_stations
//---------------Connect define-------------
#define CONTROL_BYTE_CON         0x10                       // Control Bye of Publish
#define PACKET_LEN_CON           0x12                       // Length of Packet
#define PRO_NAME_LEN_MSB_CON     0x00                       // Protocol Name Length (MSB)
#define PRO_NAME_CON             "MQTT"                     // Protocol Name
#define PRO_TCP_IPV4             0x04                       // TCP : IPV4
#define FLAG_CONNCECT            0x02                       // Connect Flags
#define KEEP_ALIVE_MSB           0x00                       // Keep Live (MSB)
#define KEEP_ALIVE_LSB           0x3C                       // Keep Live (LSB)
#define CLIENT_IDEN_LEN_MSB      0x00                       // Client Identifier Length (MSB)
#define CLIENT_NAME_CON          "ABCDEF"                   // Client name
//---------------Publish define-------------
#define CONTROL_BYTE_PUB         0x30                       // Control Bye of Publish
#define PRO_NAME_LEN_MSB_PUB     0x00                       // Protocol Name Length (MSB)

#define ENTER 0x1A                                          // Ky tu Enter
#use fast_io(F)                                             // Ket noi giua DHT22 voi port F

float UV;
float Dust;
float PH;
float tmp, hum;

//-------------random------------------------
int random(int minN, int maxN){
    return minN + rand() % (maxN + 1 - minN);
}

//******************AT COMMANDS**************************************
void AT_Commands(){
   printf("AT+CIPCLOSE");                         
   putc(13,UART_PORT2); 
   delay_ms(500);

   printf("AT");                             
   putc(13,UART_PORT2);
   delay_ms(500);

   printf("AT+CREG?");                     
   putc(13,UART_PORT2);
   delay_ms(500);

   printf("AT+CGATT?");                      
   putc(13,UART_PORT2);
   delay_ms(500);

   printf("AT+CIPSHUT"); 
   putc(13,UART_PORT2);
   delay_ms(500);

   printf("AT+CSTT=\"www\",\"\",\"\" "); 
   putc(13,UART_PORT2);
   delay_ms(1000);

   printf("AT+CIICR"); 
   putc(13,UART_PORT2);
   delay_ms(500);

   printf("AT +CIFSR"); 
   putc(13,UART_PORT2);
   delay_ms(500);
}

//*********************CONNCET***************************************
void connect(){
   //char command[256];
   char command_con[256], command_client[256];
   char broker_command[256];
   
   sprintf(broker_command,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"",BROKER_ADDR,BROKER_PORT);
   printf("%s",broker_command); 
   putc(13,UART_PORT2);
   delay_ms(3500);
   printf("AT+CIPSEND;"); 
   putc(13,UART_PORT2);
   delay_ms(50);
  
   sprintf(command_con,"%s",(char*)PRO_NAME_CON);
   sprintf(command_client,"%s",(char*)CLIENT_NAME_CON);
   printf("%c%c%c%c%s%c%c%c%c%c%c%s%c",(char*)CONTROL_BYTE_CON,(char)(strlen(command_con) + strlen(command_client) + 8),(char*)PRO_NAME_LEN_MSB_CON,(char)strlen((char*)PRO_NAME_CON),command_con,(char*)PRO_TCP_IPV4,(char*)FLAG_CONNCECT,(char*)KEEP_ALIVE_MSB,(char*)KEEP_ALIVE_LSB,(char*)CLIENT_IDEN_LEN_MSB,(char)strlen(command_client),command_client,ENTER);   
}

//***********************PUBLISH***********************************
void send_data(){
   char command[256];
   // float tmp1, hum1, ph1, dust1, uv1;
   
   // tmp  = 1;                           // Bien nhiet do
   // hum  = 1;                           // Bien do am
   // PH   = 1;                            // Bien pH
   // Dust = 1;                          // Bien bui
   // UV   = 1;                            // Bien Uv

   UV = read_avg_UV();
      
   //Read TMP AND HUM value
   Start_signal();
   if(!check_response()) 
   continue;
   RH_Byte1 = Read_Data();
   RH_Byte2 = Read_Data();
   T_Byte1  = Read_Data();
   T_Byte2  = Read_Data();    
   Read_Data(); // Checksum 
   // byte dau tien Tmp*(FF + 1) + byte thu 2 Tmp;  
   tmp = ((float)T_Byte1*256+ (float)T_Byte2) / 10;    
   hum = ((float)RH_Byte1*256+ (float)RH_Byte2) / 10;   
   
   //Read DUST value
   Dust = read_avg_Dust(); 
   if(Dust < 0) Dust = 0;
   
   //Read PH value
   PH = read_avg_PH();
   
   printf("AT+CIPSEND;");                 // Yeu cau gui du lieu
   putc(13,UART_PORT2); 
   delay_ms(500);
   sprintf(command,"%s{\"DEVICE_ID\":\"demo_station\",\"TMP\":%f,\"HUM\":%f,\"UV\":%f,\"DUST\":%f,\"PH\":%f}",(char*)BROKER_TOPIC, tmp, hum, UV, Dust, PH);            // Data publich message
   printf("%c%c%c%c%s%c",(char*)CONTROL_BYTE_PUB,(char)(strlen(command) + 2),(char*)PRO_NAME_LEN_MSB_PUB,(char)strlen((char*)BROKER_TOPIC),command,ENTER);
}

//***********************Debug***************************************
void DB_Log (char* log)
{
   puts(log,UART_PORT1);
   putc(13,UART_PORT2);
}

//***********************MAIN***************************************
void main(){
   
//----Khoi tao Analog---//
   ana_init();
   delay_ms(1000);
   
//----Module_SIM-------//
   int dem = 0;
   DB_Log((char*)"=======JWCLAB START========");
   AT_Commands();
   delay_ms(50);

   DB_Log((char*)"=======JWCLAB CONNECT BROKER========");
   
   connect();
   delay_ms(200);
   
   
   while(TRUE)
   {        
      DB_Log((char*)"=======JWCLAB SEND DATA BROKER========");
      send_data(); 
      delay_ms(15000);
      dem++;
      if(dem == 1000){
          DB_Log((char*)"=======JWCLAB CLOSE BROKER========");
          printf("AT+CIPCLOSE");
          putc(13,UART_PORT2);
          delay_ms(3000);
          DB_Log((char*)"=======JWCLAB CONNECT BROKER========");
          connect();delay_ms(50);
          dem = 0;
      }    
   }
}
