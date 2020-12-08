#BIT Data_1 = 0x02E0.1                       // Pin mapped to PORTF.1
#BIT DataDir_1 = 0x02DE.1                    // Pin direction mapped to TRISF.1

unsigned short k;
unsigned short T_Byte1, T_Byte2, RH_Byte1, RH_Byte2;

void start_signal(){
  DataDir_1 = 0;    // output cua DSPIC30  // send tin hieu request  
  Data_1    = 0; 
  Delay_ms(25);
  Data_1    = 1; 
  Delay_us(30);
  DataDir_1 = 1;    // input cua DSPIC30
}
unsigned short check_response(){   // kiem tra trong khoang thoi gian 300us data = 1 or 0 th� tra ve 0 => data thay doi lien tuc
  k = 150;
  while(!Data_1){
    Delay_us(2);
    k--;
    if(k<1) return 0;  // time out
  }
  k = 150;
  while(Data_1){
    Delay_us(2);
    k--;
    if(k<1) return 0;   // time out
  }
  return 1;
}
unsigned short Read_Data(){
  int i;
  unsigned short num = 0;
  DataDir_1 = 1;             // DSPIC nhan du lieu tai Data pin
  for (i=0; i<8; i++){
    while(!Data_1);
    Delay_us(40);
    if(Data_1) num |= 1<<(7-i);
    while(Data_1);
  }
  return num;
}