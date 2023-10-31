// master
#include <SPI.h>

const int SLAVE_PIN1 = 2;
const int SLAVE_PIN2 = 3;
const int SLAVE_PIN3 = 4;
const int SLAVE_PIN4 = 5;

void setup() {
  Serial.begin(9600);
  pinMode(11, OUTPUT);
  pinMode(12, INPUT);
  pinMode(13, OUTPUT);
  pinMode(SLAVE_PIN1, OUTPUT);
  pinMode(SLAVE_PIN2, OUTPUT);
  pinMode(SLAVE_PIN3, OUTPUT);
  digitalWrite(SLAVE_PIN1, HIGH); // 슬레이브 해제
  digitalWrite(SLAVE_PIN2, HIGH); // 슬레이브 해제
  digitalWrite(SLAVE_PIN3, HIGH); // 슬레이브 해제
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8); 
  // digitalWrite(SLAVE_PIN4, HIGH); // 슬레이브 해제

}


void loop() {
  // 슬레이브 1 통신
  String data1 = "";
  digitalWrite(SLAVE_PIN1, LOW);
  delay(100);
  while(1) {
    char received = SPI.transfer(0);
    if(received == '\0') {
      break;
    } else {
      data1 += received;
    }
  }
  digitalWrite(SLAVE_PIN1, HIGH);
  if(data1 != "") {
    Serial.print("slave 1 :");
    Serial.println(data1);
  }
  

  String data2 = "";
  digitalWrite(SLAVE_PIN2, LOW);
  delay(100);
  while(1) {
    char received = SPI.transfer(0);
    if(received == '\0') {
      break;
    } else {
      data2 += received;
    }
  }
  digitalWrite(SLAVE_PIN2, HIGH);
  if(data2 != "") {
    Serial.print("slave 2 :");
    Serial.println(data2);
  }
  

  String data3 = "";
  digitalWrite(SLAVE_PIN3, LOW);
  delay(100);
  while(1) {
    char received = SPI.transfer(0);
    if(received == '\0') {
      break;
    } else {
      data3 += received;
    }
  }
  digitalWrite(SLAVE_PIN3, HIGH);
  if(data3 != "") {
    Serial.print("slave 3 :");
    Serial.println(data3);
  }

 
}

==========================================
// slave1
// pin 1
#include <DHT.h>  // DHT22 센서용 라이브러리
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>

#define slaveNumber 'C'

// DHT22 센서 초기화
#define DHTPIN 9           // DHT센서값을 받을 디지털핀
#define DHTTYPE DHT22      // 사용할 센서타입을 DHT22 로 지정
DHT dht(DHTPIN, DHTTYPE);  // 센서 객체 생성

#define PIN_TX_PMS7003 7  // PIN matched with TX of PMS7003
#define PIX_RX_PMS7003 6   // PIN matched with RX of PMS7003

#define HEAD_1 0x42
#define HEAD_2 0x4d

#define PMS7003_BAUD_RATE 9600  // Serial Speed of PMS7003

SoftwareSerial pmsSerial(PIN_TX_PMS7003, PIX_RX_PMS7003);  // RX, TX of Arduino UNO
unsigned char pmsbytes[32];                                // array for 32 bytes stream from PMS7003

#define SLAVE_ADDRESS 0x59
#define SLAVE_ADDRESS_WRITE 0xB2
#define SLAVE_ADDRESS_READ 0xB3

unsigned long previousTime1 = 0;       // 첫 번째 함수 호출 시간 기록
unsigned long previousTime2 = 0;       // 두 번째 함수 호출 시간 기록
const unsigned long interval1 = 1000;  // 첫 번째 함수 호출 주기 (1000ms)
const unsigned long interval2 = 2000;  // 두 번째 함수 호출 주기 (2000ms)

uint16_t pm1_0;
uint16_t pm2_5;
uint16_t pm10;


float h;
float t;
float Vol[4] = {
  0,
};

String getI2c;

byte count;
byte buffer[500];
bool sendState;
bool interruptState;
void setup() {

  count = 0;
  sendState = false;
  interruptState = true;
  Wire.begin();
  pmsSerial.begin(PMS7003_BAUD_RATE);
  dht.begin();
  Serial.begin(9600);

  pinMode(11, INPUT);
  pinMode(13, INPUT);
  pinMode(12, OUTPUT); 

  SPI.setClockDivider(SPI_CLOCK_DIV8); 
  SPCR |=_BV(SPE);
  SPCR &= ~_BV(MSTR);
  SPCR|=_BV(SPIE);

}

ISR (SPI_STC_vect) {
  if(interruptState) {
    sendState = true;

    if(buffer[count] != '\0') {
      SPDR = buffer[count];
      count++;
    } else if(buffer[count] == '\0') {
      SPDR = '\0';
      count = 0;
      sendState = false;
    } 
  } else {
    SPDR = '\0';
  }
}

void loop() {
  unsigned long currentTime = millis();  // 현재 시간 읽기
  if ((pmsSerial.available() >= 32) && (!sendState)) {
    interruptState = false;
    int i = 0;

    //initialize first two bytes with 0x00
    pmsbytes[0] = 0x00;
    pmsbytes[1] = 0x00;

    for (i = 0; i < 32; i++) {
      pmsbytes[i] = pmsSerial.read();

      //check first two bytes - HEAD_1 and HEAD_2, exit when it's not normal and read again from the start
      if ((i == 0 && pmsbytes[0] != HEAD_1) || (i == 1 && pmsbytes[1] != HEAD_2)) {
        break;
      }
    }

    if (i > 2) {                                     // only when first two stream bytes are normal
      if (pmsbytes[29] == 0x00) {                    // only when stream error code is 0
        pm1_0 = (pmsbytes[10] << 8) | pmsbytes[11];  // pmsbytes[10]:HighByte + pmsbytes[11]:LowByte => two bytes
        pm2_5 = (pmsbytes[12] << 8) | pmsbytes[13];  // pmsbytes[12]:HighByte + pmsbytes[13]:LowByte => two bytes
        pm10 = (pmsbytes[14] << 8) | pmsbytes[15];   // pmsbytes[14]:HighByte + pmsbytes[15]:LowByte => two bytes

      } else {
        // Serial.println("Error skipped..");
      }
    } else {
      // Serial.println("Bad stream format error");
    }
    interruptState = true;
  }

  if ((currentTime - previousTime1 >= interval1) && (!sendState)) {
    interruptState = false;
    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write(SLAVE_ADDRESS_WRITE);
    Wire.write(0x52);
    Wire.endTransmission();

    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write(SLAVE_ADDRESS_READ);
    Wire.endTransmission();

    Wire.requestFrom(SLAVE_ADDRESS, 7);  // 7바이트 데이터 요청
    if (Wire.available() >= 7) {
      byte data[7];
      for (int i = 0; i < 7; i++) {
        data[i] = Wire.read();
      }
      int value = (data[1] << 8) | data[2];
      getI2c = String(value);
    }
    previousTime1 = currentTime;  // 다음 호출 시간 갱신
    interruptState = true;
  }

  if ((currentTime - previousTime2 >= interval2) && (!sendState)) {
    interruptState = false;
    h = dht.readHumidity();     // 습도값 요청
    t = dht.readTemperature();  // 온도값 요청

    for(int i = 0; i < 4; i++) {
      Vol[i] = (analogRead(i) * 5.0 )/ 1024.0;
    }

    String stringData = String(slaveNumber) + "/" 
    + String(h) + "/" 
    + String(t) + "/" 
    + String(pm1_0) + "/"
    + String(pm2_5) + "/"
    + String(pm10) + "/"
    + String(getI2c) + "/"
    + String(Vol[2]) + "/"
    + String(Vol[1]) + "/"
    + String(Vol[0]) + "/"
    + String(Vol[3]) + '\0';
    stringData.toCharArray(buffer, stringData.length()+1);

    previousTime2 = currentTime;  // 다음 호출 시간 갱신
    interruptState = true;
  } 
  
   
  

    // Humidity / Temperature / pm1.0 / pm2.5 / pm10 / ELT / Mics / CJMCU / MQ / HCHO   
      
    
  

}
