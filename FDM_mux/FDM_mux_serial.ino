#include <SoftwareSerial.h>
#include <DHT.h>  // DHT22 센서용 라이브러리

SoftwareSerial mySerial(13, 15);  // RX, TX
unsigned char pms[32];

// DHT22 센서 초기화
#define DHTPIN 12          // DHT센서값을 받을 디지털핀
#define DHTTYPE DHT22      // 사용할 센서타입을 DHT22 로 지정
DHT dht(DHTPIN, DHTTYPE);  // 센서 객체 생성

//Mux control pins
int s0 = 5;
int s1 = 4;
int s2 = 14;

//Mux in "Z" pin
int Z_pin = 0;

int PM1_0;
int PM2_5;
int PM10;
float h;
float t;
float Vol[5] = {
  0,
};


float readMux(int channel);

void setup() {
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);

  Serial.begin(115200);
  mySerial.begin(9600);
  dht.begin();
}

void loop() {

  if (mySerial.available() >= 32) {

    for (int j = 0; j < 32; j++) {
      pms[j] = mySerial.read();
    }
    PM1_0 = (pms[10] << 8) | pms[11];
    PM2_5 = (pms[12] << 8) | pms[13];
    PM10 = (pms[14] << 8) | pms[15];

    static uint32_t started = 0;  // 타이머용 변수 선언 및 초기화
    if (millis() - started >= 2000) {
      started = millis();
      h = dht.readHumidity();     // 습도값 요청
      t = dht.readTemperature();  // 온도값 요청

      for (int i = 0; i < 5; i++) {
        Vol[i] = readMux(i);
      }

      Serial.printf("%.2f/%.2f/%d/%d/%d/%.2f/%.2f/%.2f/%.2f/%.2f/ \n", h, t, PM1_0, PM2_5, PM10, Vol[0], Vol[1], Vol[2], Vol[3], Vol[4]);
      // Vol[0]: PID, Vol[1]: MiCS, Vol[2]: CJMCU, Vol[3]: MQ, Vol[4]: HCHO
      
      // Serial.print(h);
      // Serial.print("/");
      // Serial.print(t);
      // Serial.print("/");
      // Serial.print(PM1_0);
      // Serial.print("/");
      // Serial.print(PM2_5);
      // Serial.print("/");
      // Serial.print(PM10);
      // Serial.print("/");
      // Serial.print(Vol[0]);
      // Serial.print("/");
      // Serial.print(Vol[1]);
      // Serial.print("/");
      // Serial.print(Vol[2]);
      // Serial.println("/");

    }
  }
}


float readMux(int channel) {
  int controlPin[] = { s0, s1, s2 };

  int muxChannel[8][3] = {
    { 0, 0, 0 },  //channel 0
    { 1, 0, 0 },  //channel 1
    { 0, 1, 0 },  //channel 2
    { 1, 1, 0 },  //channel 3
    { 0, 0, 1 },  //channel 4
    { 1, 0, 1 },  //channel 5
    { 0, 1, 1 },  //channel 6
    { 1, 1, 1 },  //channel 7
  };

  //loop through the 3 sig
  for (int i = 0; i < 3; i++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the Z pin
  int val = analogRead(Z_pin);

  //return the value
  float voltage = (val * 3.286) / 1024.0;
  return voltage;
}
