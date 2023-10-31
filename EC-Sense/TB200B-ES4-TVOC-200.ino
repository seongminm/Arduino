// passive mode_Edit Ver
#include <SoftwareSerial.h>

  // put your setup code here, to run once:
  SoftwareSerial mySerial(13, 12); //Uno Rx Tx (13 12) = mySerial
  
  // Switch to passive upload.
  byte Passive_mode[9] = {0xFF, 0x01, 0x78, 0x41, 0x00, 0x00, 0x00, 0x00, 0x46};  

  // Gas concentration value and temperature and humidity combined reading instruction
  byte Gas_th_value_request[9] = {0xFF, 0x00, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79}; 

void setup() {
  Serial.begin(9600); 
  mySerial.begin(9600);  
  delay(1000); 
  while(!mySerial){} 
  mySerial.write(Passive_mode, 9);
}

void loop() {
  unsigned char receive_data[13] = { 0x00, };
  mySerial.write(Gas_th_value_request, 9);
  delay(500);
  int packetIndex = 0;
  while(mySerial.available()>0){ 
    int ch = mySerial.read();
    receive_data[packetIndex] = ch;
    Serial.print(ch, HEX);
    Serial.print(' ');
    packetIndex += 1;
  }
  
  // checksum
  if( (packetIndex == 13) &&(1 + (0xFF ^ (byte)(receive_data[1] + receive_data[2] + receive_data[3] 
  + receive_data[4] + receive_data[5] + receive_data[6] + receive_data[7] + receive_data[8] + receive_data[9] + receive_data[10] + receive_data[11]))) == receive_data[12])
  {
    float VOC_value = (float)(int)((receive_data[6] * 256) + receive_data[7]) / 100;
    float T_value = (float)(int)(receive_data[8] << 8 | receive_data[9]) / 100;
    float H_value = (float)(int)(receive_data[10] << 8 | receive_data[11]) / 100;
    Serial.println("");
    Serial.print("VOC : ");
    Serial.print(VOC_value);
    Serial.print(" TEM : ");
    Serial.print(T_value);
    Serial.print(" HUM : ");
    Serial.println(H_value);
    Serial.println("--------------------");
  }
  

}
