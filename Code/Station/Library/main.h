#include <30F6014.h>
#device ADC=12
#device ICSP=1
#use delay(crystal=8000000)

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES CKSFSM                   //Clock Switching is enabled, fail Safe clock monitor is enabled
#FUSES NOBROWNOUT               //No brownout reset


#use STANDARD_IO( F )
#use rs232(UART1, baud=9600, stream=UART_PORT1)
#use rs232(UART2, baud=9600, stream=UART_PORT2)


