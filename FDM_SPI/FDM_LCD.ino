// master

#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const int SLAVE_PIN1 = 2;

LiquidCrystal_I2C lcd(0x3F, 16, 2);

String prePm10, preVoc = "";

void setup() {
  pinMode(11, OUTPUT);
  pinMode(12, INPUT);
  pinMode(13, OUTPUT);
  pinMode(SLAVE_PIN1,OUTPUT);
  digitalWrite(SLAVE_PIN1, HIGH); // 슬레이브 해제

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8); 

	lcd.init();
	lcd.backlight();

  Serial.begin(9600);

  lcd.setCursor(0,0);
  lcd.print(" PM1.0 : ");

  lcd.setCursor(0,1);
  lcd.print("  VOC  : ");
}

void loop() { 
	// random 함수를 사용하여 0부터 2까지의 숫자중 랜덤하게 값을 받습니다.
  
  String getString = "";
  digitalWrite(SLAVE_PIN1, LOW); 
  delay(100);
  while(1) {
    char received1 = SPI.transfer(0);
    if(received1 == '\0') {
      break;
    } else {
      getString += received1;
    }
  }
  digitalWrite(SLAVE_PIN1, HIGH);

  if(getString != "") {
    int stringLength = getString.length();
    char charArray[stringLength];
    getString.toCharArray(charArray, stringLength + 1);

    if(charArray[0] = 'A') {
      String pm10 = "";
      String voc = "";
      int findIndex = 0;
      
      for(int i = 2; i < stringLength; i++) {
        if(charArray[i] != '/') {
          pm10 += charArray[i];
        } else {
          findIndex = i;
          break;
        }
      }

      for(int j = findIndex +1; j < stringLength; j++) {
        voc += charArray[j];
      }

      if((pm10 != "") && (voc != "")) {
        Serial.println(getString);

        if(prePm10 != pm10) {
          prePm10 = pm10;

          lcd.setCursor(9,0);
          lcd.print("     ");

          lcd.setCursor(9,0);
          lcd.print(prePm10);
        }

        if(preVoc != voc) {
          preVoc = voc;

          lcd.setCursor(9,1);
          lcd.print("     ");

          lcd.setCursor(9,1);
          lcd.print(preVoc);
        }

        delay(1000);
      }
    } 
  }
}

===============================================

// slave

#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>

#define slaveNumber 'A'

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

String getI2c;

String stringData;
int bufferSize;

void setup() {
  // put your setup code here, to run once:
  stringData = "";
  bufferSize = 0;
  Wire.begin();
  pmsSerial.begin(PMS7003_BAUD_RATE);
  
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(13, INPUT);
  pinMode(12, OUTPUT); 

  SPI.setClockDivider(SPI_CLOCK_DIV8); 
  SPCR |=_BV(SPE);
  SPCR &= ~_BV(MSTR);
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long currentTime = millis();  // 현재 시간 읽기
  if (pmsSerial.available() >= 32) {
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
  }

  if (currentTime - previousTime1 >= interval1) {
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
    stringData = String(slaveNumber) + "/" + String(pm1_0) + "/" + String(getI2c) + '\0';
    bufferSize = stringData.length()+1;

    previousTime1 = currentTime;  // 다음 호출 시간 갱신
  }

  

  if(digitalRead(10) == LOW) {
    byte buffer[bufferSize];

    stringData.toCharArray(buffer, bufferSize);
    
    for(int i = 0; i < bufferSize; i++) {
      SPI.transfer(buffer[i]);
    }      
  } 
    // Humidity / Temperature / pm1.0 / pm2.5 / pm10 / ELT / Mics / CJMCU / MQ / HCHO 
}
