#define ADC000  0
#define ADC12V  2605.0

//--------khai bao bien UV ------------------//
float read_avg_UV ();
float read_single_UV ();

//--------khoi tao analog-------------------//
void ana_init() {
   setup_adc_ports(sAN2 | sAN3, VSS_VDD);
   setup_adc(ADC_CLOCK | ADC_TAD_MUL_0);
}

//--------doc analog-----------------------//
int16  read_analog() {
   int16 value;
   float tmp;
   set_adc_channel(2);
   delay_us(10);
   value = read_adc();
   tmp = (value * 1200.0) / ADC12V;
   value = (int16) floor (tmp);   

   return value;  
}

float read_avg_UV ()
{
   float uv_buff[10];
   float temp;
   float avgValue;
   
      // read 10 sample
     for (int k = 0; k < 10; k++){
         uv_buff[k] = read_single_UV();
      }
      // sort 10 sample
      for (int i = 0; i < 9; i++){
         for (int j = i + 1; j < 10; j++){
            if (uv_buff[i] > uv_buff[j]){
               temp = uv_buff[i];
               uv_buff[i] = uv_buff[j];
               uv_buff[j] = temp;
            }
         }
      }      
      // avg caculation
      avgValue = 0;
      for (int n = 2; n < 8; n++)
         avgValue += uv_buff[n];
  
      return avgValue/6;
}

float read_single_UV () 
{
      int ADC_value;
      float ADC_float_value;
      float UV_Result;
      
      ADC_value = read_analog();
      ADC_float_value = (ADC_value - (float)((int)ADC_value%100))/100 + (float)((int)ADC_value%100)/100; 
      UV_Result = 8*ADC_float_value - 8;  
         
      delay_ms(5);
      
      return UV_Result;

}
