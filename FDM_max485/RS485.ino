// 송신

#include <SoftwareSerial.h>
SoftwareSerial rs485(2, 3); // RX, TX pins
#define RS485Control 7
byte id = 1;

void setup() {
  Serial.begin(9600);
  rs485.begin(9600);
  pinMode(RS485Control, OUTPUT);
  digitalWrite(RS485Control, HIGH); // 송신모드  
}

void loop() {
    digitalWrite(RS485Control, HIGH); // 송신모드
    static uint32_t started = 0;  // 타이머용 변수 선언 및 초기화
    rs485.write(id);
    digitalWrite(RS485Control, LOW); // 수신모드
    delay(10);
    while(1) {
      if(rs485.available()) {
        String data = rs485.readStringUntil('\n');
        Serial.println(data);
        id++;
        break;
      } else if(millis() - started >= 1000) {
        Serial.println("1초 무답");
        break;
      }
    }
    started = millis();

    if(id>3) {
      id = 1;
      delay(2000);
    }
}
    
// slave 1

//수신

#include <SoftwareSerial.h>
#include <DHT.h>  // DHT22 센서용 라이브러리

SoftwareSerial rs485(2, 3); // RX, TX pins
#define RS485Control 7
#define slave1 0x00 //0
unsigned char pmsbytes[32]; // array for 32 bytes stream from PMS7003
#define HEAD_1 0x42
#define HEAD_2 0x4d


// DHT22 센서 초기화
#define DHTPIN 9          // DHT센서값을 받을 디지털핀
#define DHTTYPE DHT22      // 사용할 센서타입을 DHT22 로 지정
DHT dht(DHTPIN, DHTTYPE);  // 센서 객체 생성


uint16_t pm1_0;
uint16_t pm2_5;
uint16_t pm10;

float h;
float t;

String data;

// String data;

void setup() {
  Serial.begin(9600);
  rs485.begin(9600);
  pinMode(RS485Control, OUTPUT);
  dht.begin();
  digitalWrite(RS485Control, LOW); // 수신모드
}

void loop() {
  if(Serial.available()>=32){
    int i=0;

    //initialize first two bytes with 0x00
    pmsbytes[0] = 0x00;
    pmsbytes[1] = 0x00;
    
    for(i=0; i<32 ; i++){
      pmsbytes[i] = Serial.read();

      //check first two bytes - HEAD_1 and HEAD_2, exit when it's not normal and read again from the start
      if( (i==0 && pmsbytes[0] != HEAD_1) || (i==1 && pmsbytes[1] != HEAD_2) ) {
        break;
      }
    }

    if(i>2) { // only when first two stream bytes are normal
      if(pmsbytes[29] == 0x00) {  // only when stream error code is 0
         pm1_0 = (pmsbytes[10]<<8) | pmsbytes[11]; // pmsbytes[10]:HighByte + pmsbytes[11]:LowByte => two bytes
         pm2_5 = (pmsbytes[12]<<8) | pmsbytes[13]; // pmsbytes[12]:HighByte + pmsbytes[13]:LowByte => two bytes
         pm10 = (pmsbytes[14]<<8) | pmsbytes[15]; // pmsbytes[14]:HighByte + pmsbytes[15]:LowByte => two bytes

      } else {
        // Serial.println("Error skipped..");
      }
    } else {
      // Serial.println("Bad stream format error");
    }
  }

  static uint32_t started = 0;  // 타이머용 변수 선언 및 초기화
    if (millis() - started >= 2000) {
      started = millis();
      h = dht.readHumidity();     // 습도값 요청
      t = dht.readTemperature();  // 온도값 요청
      data= String(h) + "/" + String(t) + "/" + String(pm1_0) + "/" + String(pm2_5) + "/" + String(pm10) + '\n'; 
      
    }


      if(rs485.read() == 1) {
        digitalWrite(RS485Control, HIGH);
        rs485.print(1);
        rs485.print("/");
        rs485.print(data);
        digitalWrite(RS485Control, LOW);
      } else {
        return;
      }
}

// slave 2
//수신

#include <SoftwareSerial.h>
#include <DHT.h>  // DHT22 센서용 라이브러리

SoftwareSerial rs485(2, 3); // RX, TX pins
#define RS485Control 7
#define slave1 0x00 //0
unsigned char pmsbytes[32]; // array for 32 bytes stream from PMS7003
#define HEAD_1 0x42
#define HEAD_2 0x4d


// DHT22 센서 초기화
#define DHTPIN 9          // DHT센서값을 받을 디지털핀
#define DHTTYPE DHT22      // 사용할 센서타입을 DHT22 로 지정
DHT dht(DHTPIN, DHTTYPE);  // 센서 객체 생성


uint16_t pm1_0;
uint16_t pm2_5;
uint16_t pm10;

float h;
float t;

String data;

void setup() {
  Serial.begin(9600);
  rs485.begin(9600);
  pinMode(RS485Control, OUTPUT);
  dht.begin();
  digitalWrite(RS485Control, LOW); // 수신모드
}

void loop() {
  if(Serial.available()>=32){
    int i=0;

    //initialize first two bytes with 0x00
    pmsbytes[0] = 0x00;
    pmsbytes[1] = 0x00;
    
    for(i=0; i<32 ; i++){
      pmsbytes[i] = Serial.read();

      //check first two bytes - HEAD_1 and HEAD_2, exit when it's not normal and read again from the start
      if( (i==0 && pmsbytes[0] != HEAD_1) || (i==1 && pmsbytes[1] != HEAD_2) ) {
        break;
      }
    }

    if(i>2) { // only when first two stream bytes are normal
      if(pmsbytes[29] == 0x00) {  // only when stream error code is 0
         pm1_0 = (pmsbytes[10]<<8) | pmsbytes[11]; // pmsbytes[10]:HighByte + pmsbytes[11]:LowByte => two bytes
         pm2_5 = (pmsbytes[12]<<8) | pmsbytes[13]; // pmsbytes[12]:HighByte + pmsbytes[13]:LowByte => two bytes
         pm10 = (pmsbytes[14]<<8) | pmsbytes[15]; // pmsbytes[14]:HighByte + pmsbytes[15]:LowByte => two bytes

      } else {
        // Serial.println("Error skipped..");
      }
    } else {
      // Serial.println("Bad stream format error");
    }
  }

  static uint32_t started = 0;  // 타이머용 변수 선언 및 초기화
    if (millis() - started >= 2000) {
      started = millis();
      h = dht.readHumidity();     // 습도값 요청
      t = dht.readTemperature();  // 온도값 요청

       data= String(h) + "/" + String(t) + "/" + String(pm1_0) + "/" + String(pm2_5) + "/" + String(pm10) + '\n'; 
    }


      if(rs485.read() == 2) {
        digitalWrite(RS485Control, HIGH);
        rs485.print(2);
        rs485.print("/");
        rs485.print(data);
        digitalWrite(RS485Control, LOW);
      } else {
        return;
      }
}

// slave 3
//수신

#include <SoftwareSerial.h>
#include <DHT.h>  // DHT22 센서용 라이브러리

SoftwareSerial rs485(2, 3); // RX, TX pins
#define RS485Control 7
unsigned char pmsbytes[32]; // array for 32 bytes stream from PMS7003
#define HEAD_1 0x42
#define HEAD_2 0x4d
// DHT22 센서 초기화
#define DHTPIN 9          // DHT센서값을 받을 디지털핀
#define DHTTYPE DHT22      // 사용할 센서타입을 DHT22 로 지정
DHT dht(DHTPIN, DHTTYPE);  // 센서 객체 생성


uint16_t pm1_0;
uint16_t pm2_5;
uint16_t pm10;

float h;
float t;

String data;

void setup() {
  Serial.begin(9600);
  rs485.begin(9600);
  pinMode(RS485Control, OUTPUT);
  dht.begin();
  digitalWrite(RS485Control, LOW); // 수신모드
}

void loop() {
  if(Serial.available()>=32){
    int i=0;

    //initialize first two bytes with 0x00
    pmsbytes[0] = 0x00;
    pmsbytes[1] = 0x00;
    
    for(i=0; i<32 ; i++){
      pmsbytes[i] = Serial.read();

      //check first two bytes - HEAD_1 and HEAD_2, exit when it's not normal and read again from the start
      if( (i==0 && pmsbytes[0] != HEAD_1) || (i==1 && pmsbytes[1] != HEAD_2) ) {
        break;
      }
    }

    if(i>2) { // only when first two stream bytes are normal
      if(pmsbytes[29] == 0x00) {  // only when stream error code is 0
         pm1_0 = (pmsbytes[10]<<8) | pmsbytes[11]; // pmsbytes[10]:HighByte + pmsbytes[11]:LowByte => two bytes
         pm2_5 = (pmsbytes[12]<<8) | pmsbytes[13]; // pmsbytes[12]:HighByte + pmsbytes[13]:LowByte => two bytes
         pm10 = (pmsbytes[14]<<8) | pmsbytes[15]; // pmsbytes[14]:HighByte + pmsbytes[15]:LowByte => two bytes

      } else {
        // Serial.println("Error skipped..");
      }
    } else {
      // Serial.println("Bad stream format error");
    }
  }

  static uint32_t started = 0;  // 타이머용 변수 선언 및 초기화
    if (millis() - started >= 2000) {
      started = millis();
      h = dht.readHumidity();     // 습도값 요청
      t = dht.readTemperature();  // 온도값 요청

       data= String(h) + "/" + String(t) + "/" + String(pm1_0) + "/" + String(pm2_5) + "/" + String(pm10) + '\n'; 
    }


      if(rs485.read() == 3) {
        digitalWrite(RS485Control, HIGH);
        rs485.print(3);
        rs485.print("/");
        rs485.print(data);
        digitalWrite(RS485Control, LOW);
      } else {
        return;
      }
}

    

  

