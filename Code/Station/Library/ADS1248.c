#ifndef ADC_ADS1248

#define ADS1248_SCLK  PIN_F6
#define ADS1248_DIN   PIN_F8
#define ADS1248_DOUT  PIN_F7
#define ADS1248_DRDY  PIN_A14
#define ADS1248_START PIN_G3
#define ADS1248_RESET PIN_G2
#define ADS1248_CS    PIN_A15

#endif

#use spi(MASTER, SPI1, BAUD=500000, MODE=2, BITS=8, stream=SPI_PORT1)

#define MUX0      0x00     //01
#define VBIAS     0x01
#define MUX1      0x02
#define SYS0      0x03
#define OFC0      0x04
#define OFC1      0x05
#define OFC2      0x06
#define FSC0      0x07
#define FSC1      0x08     //1B
#define FSC2      0x09     //40
#define IDAC0     0x0A     //90
#define IDAC1     0x0B     //FF
#define GPIOCFG   0x0C
#define GPIODIR   0x0D
#define GPIODAT   0x0E

#define ADC_WAKEUP   0x00
#define ADC_SLEEP    0x02
#define ADC_SYNC     0x04     // 2nd byte is 0x04, 0x05
#define ADC_RESET    0x06
#define ADC_NOP      0xFF
#define ADC_RDATA    0x12
#define ADC_RDATAC   0x14
#define ADC_SDATAC   0x16
#define ADC_RREG     0x20     // 2nd byte is number of registers to be read (number of bytes – 1)
#define ADC_WREG     0x40     // 2nd byte is number of registers to be written (number of bytes – 1)
#define ADC_SYSOCAL  0x60
#define ADC_SYSGCAL  0x61
#define ADC_SELFOCAL 0x62

#define LED PIN_C4                                          // Chan led cua DUST

//---khai bao bien cua Dust---//
float read_avg_Dust ();
float read_single_Dust ();

//---khai bao bien cua PH---//
float read_avg_PH ();
float read_single_PH ();

void ads1248_hard_reset() {
   output_low(ADS1248_RESET);
   delay_us(4);
   output_high(ADS1248_RESET);
   delay_ms(2);
}

void ads1248_soft_reset() {
   output_low(ADS1248_CS);
   spi_write(ADC_RESET);
   delay_ms(2);
   output_high(ADS1248_CS);
   delay_us(2);
}

void Read_all_reg() {
   int8 i, data;
   output_low(ADS1248_CS);
   output_high(ADS1248_START);
   spi_write(ADC_RREG|MUX0);     // Read from register MUX0
   spi_write(0x0D);              // 15 reg
   for(i=0;i<15;++i) {
      data = spi_read(0);
      putc(data);
   }
   output_low(ADS1248_START);
   output_high(ADS1248_CS);
}

void ads1248_init()
{
   output_low(ADS1248_CS);
   delay_ms(1);
   spi_write(ADC_RESET);
   delay_ms(2);
   output_high(ADS1248_CS);
   delay_ms(10);
   output_low(ADS1248_CS);
   spi_write(ADC_WREG|MUX0);
   spi_write(0x03);        // write 4 REG from MUX0
   spi_write(0x17);        // MUX0  00.010.111 : current source off, P=2,N=7 (connect A7 to GND)
   spi_write(0x00);        // VBIAS 0000.0000 : Bias Voltage is not set
   spi_write(0x38);        // MUX1  0.01.11.000 : ,Vref internal, Normal operation
   spi_write(0x08);        // SYS0  0.000.1000 : PGA=1,1000SPS
   spi_write(ADC_WREG|IDAC0);
   spi_write(0x01);        // write 2 REG from IDAC0
   spi_write(0x07);        // IDAC0 0000.0.111 : current source 15000uA 
   spi_write(0x77);        // IDAC1 0111.0111 : AI7, AI7
   output_high(ADS1248_CS);
   output_low(ADS1248_START);
}

void ads1248_init1()
{
   output_low(ADS1248_CS);
   delay_ms(1);
   spi_write(ADC_RESET);
   delay_ms(2);
   output_high(ADS1248_CS);
   delay_ms(10);
   output_low(ADS1248_CS);
   spi_write(ADC_WREG|MUX0);
   spi_write(0x03);        // write 4 REG from MUX0
   spi_write(0x0f);        // MUX0  00.001.111 : current source off, P=1,N=7 (connect A7 to GND)
   spi_write(0x00);        // VBIAS 0000.0000 : Bias Voltage is not set
   spi_write(0x38);        // MUX1  0.01.11.000 : ,Vref internal, Normal operation
   spi_write(0x08);        // SYS0  0.000.1000 : PGA=1,1000SPS   
   spi_write(ADC_WREG|IDAC0);
   spi_write(0x01);        // write 2 REG from IDAC0
   spi_write(0x07);        // IDAC0 0000.0.111 : current source 15000uA 
   spi_write(0x77);        // IDAC1 0111.0111 : AI7, AI7
   output_high(ADS1248_CS);
   output_low(ADS1248_START);
}
int32 convert_24bits(int32 value) {
   if(value > 0x7FFFFF) {
      return (value - 0x800000);
      //return 0;
   } else {
      return (value + 0x800000);
      //return value;
   }
}

int32 ads1248_read_adc() {
   int8 MSB, MID, LSB;
   output_low(ADS1248_CS);
   output_high(ADS1248_START);
   delay_us(2);
   output_low(ADS1248_START);
   while(input(ADS1248_DRDY) == 1);
   spi_write(ADC_RDATA);
   delay_us(1);
   MSB = spi_read(1);
   MID = spi_read(1);
   LSB = spi_read(1);
   //printf("%d %d %d \n", MSB,MID,LSB);
   output_high(ADS1248_CS);
   return convert_24bits(make32(0,MSB,MID,LSB));
}
int32 ads1248_read_adc1() {
   int8 MSB1, MID1, LSB1;
   output_low(ADS1248_CS);
   output_high(ADS1248_START);
   delay_us(2);
   output_low(ADS1248_START);
   while(input(ADS1248_DRDY) == 1);
   spi_write(ADC_RDATA);
   delay_us(1);
   MSB1 = spi_read(2);
   MID1 = spi_read(2);
   LSB1 = spi_read(2);
   //printf("%d %d %d \n", MSB,MID,LSB);
   output_high(ADS1248_CS);
   return convert_24bits(make32(0,MSB1,MID1,LSB1));
}

//---------Reset-------------//
void ADC_reset_REG()
{
   output_low(ADS1248_RESET);
   delay_us(4);
   output_high(ADS1248_RESET);
   delay_ms(2);
   output_low(ADS1248_CS);
   spi_write(ADC_RESET);
   delay_ms(2);
   output_high(ADS1248_CS);
   delay_us(2);
}

//---------DUST-------------//
float read_avg_Dust ()
{
   float dust_buff[10];
   float temp;
   float avgValue;
   
      // read 10 sample
     for (int i = 0; i < 10; i++){
         dust_buff[i] = read_single_Dust();
      }
      // sort 10 sample
      for (int i = 0; i < 9; i++){
         for (int j = i + 1; j < 10; j++){
            if (dust_buff[i] > dust_buff[j]){
               temp = dust_buff[i];
               dust_buff[i] = dust_buff[j];
               dust_buff[j] = temp;
            }
         }
      }      
      // avg caculation
      avgValue = 0;
      for (int i = 2; i < 8; i++)
         avgValue += dust_buff[i];
  
      return avgValue/6;
}

float read_single_Dust ()
{
      int32 ADC_value;
      float ADC_float_value;
      float Dust_Result;
      
      ADC_reset_REG();
      ads1248_init();
      
      output_low(LED);
      delay_us(280);
      ADC_value = ads1248_read_adc1();
      delay_us(40);
      output_high(LED);
      delay_us(9680);
      
      ADC_float_value = ADC_value / 8388608.0 * 2.048-(2.048-2.020);
      
      Dust_Result = 0.17*ADC_float_value-0.1;
     
      delay_ms(20);
      
      return Dust_Result;
}

//---------PH-------------//
float read_avg_PH ()
{
   float PH_buff[10];
   float temp;
   float avgValue;
   
      // read 10 sample
     for (int i = 0; i < 10; i++){
        PH_buff[i] = read_single_PH();
      }
      // sort 10 sample
      for (int i = 0; i < 9; i++){
         for (int j = i + 1; j < 10; j++){
            if (PH_buff[i] > PH_buff[j]){
               temp = PH_buff[i];
               PH_buff[i] = PH_buff[j];
               PH_buff[j] = temp;
            }
         }
      }      
      // avg caculation
      avgValue = 0;
      for (int i = 2; i < 8; i++)
         avgValue += PH_buff[i];
  
      return avgValue/6;
}

float read_single_PH ()
{
      int32 ADC_value;
      float ADC_float_value;
      float PH_Result;
      
      ADC_reset_REG();
      ads1248_init1();
      
      ADC_value = ads1248_read_adc();
      ADC_float_value = ADC_value / 8388608.0 * 2.048-(2.048-2.020);
      PH_Result= -5.70*ADC_float_value + 21.34;
      
     // PH_Result = 0.17*ADC_float_value-0.1;
     
      delay_ms(5);
      
      return PH_Result;

}
