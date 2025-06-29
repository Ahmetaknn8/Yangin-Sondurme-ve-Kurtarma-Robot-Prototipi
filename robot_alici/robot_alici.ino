#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

RF24 radio(9, 10); // CE, CSN
const byte adres[] = "00001";

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define IN1 2
#define IN2 3
#define IN3 4
#define IN4 5
#define ROLE_PIN 6
#define LED1 A2
#define LED2 A3
#define LED3 A6
#define BUZZER A1
#define SU_SENSOR A0

// Servo açı sınırları
int minAcilar[7] = {5, 0, 0, 40, 50, 23, 0};
int maxAcilar[7] = {175, 180, 180, 120, 130, 150, 180};

unsigned long sonVeriZamani = 0;
const unsigned long timeoutSuresi = 500;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(60);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  pinMode(ROLE_PIN, OUTPUT);
  digitalWrite(ROLE_PIN, HIGH);  // Röleyi pasif başlat (aktif-low tipi için)

  pinMode(LED1, OUTPUT); pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT); pinMode(BUZZER, OUTPUT);

  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(0, adres);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    byte veri[13];
    radio.read(&veri, sizeof(veri));
    sonVeriZamani = millis();

    // Joystick kontrollü 2 servo
    for (int i = 0; i < 2; i++) {
      int aci = map(veri[i], 0, 255, minAcilar[i], maxAcilar[i]);
      int pwmDeger = map(aci, 0, 180, 102, 512);
      pwm.setPWM(i, 0, pwmDeger);
    }

    // Potansiyometre kontrollü 5 servo (kanal 2–6)
    for (int i = 0; i < 5; i++) {
      int aci = map(veri[i + 2], 0, 255, minAcilar[i + 2], maxAcilar[i + 2]);
      int pwmDeger = map(aci, 0, 180, 102, 512);
      pwm.setPWM(i + 2, 0, pwmDeger);
    }

    // Yön kontrolü
    if (veri[7] == 1) {
      digitalWrite(IN1, HIGH); 
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);  
      digitalWrite(IN4, HIGH);
    } else if (veri[8] == 1) {
      digitalWrite(IN1, LOW);  
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH); 
      digitalWrite(IN4, LOW);
    } else if (veri[9] == 1) {
      digitalWrite(IN1, LOW);  
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);  
      digitalWrite(IN4, HIGH);
    } else if (veri[10] == 1) {
      digitalWrite(IN1, HIGH); 
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);  
      digitalWrite(IN4, LOW);
    } else {
      digitalWrite(IN1, LOW);  
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);  
      digitalWrite(IN4, LOW);
    }

    // Röle kontrolü (toggle switch)
    digitalWrite(ROLE_PIN, veri[11]);
  }

  // Zaman aşımı: veri gelmiyorsa sistem güvenli moda geçer
  if (millis() - sonVeriZamani > timeoutSuresi) {
    for (int i = 0; i < 7; i++) pwm.setPWM(i, 0, 0);
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    digitalWrite(ROLE_PIN, HIGH);  // Zaman aşımında röleyi pasif yap
  }

  // Su seviyesi sensörü
  int sensorDeger = analogRead(SU_SENSOR);
  int suYuzdesi = map(sensorDeger, 0, 1023, 0, 100);
  suYuzdesi = constrain(suYuzdesi, 0, 100);

  if (suYuzdesi > 66) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    digitalWrite(BUZZER, LOW);
  } else if (suYuzdesi > 33) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
    digitalWrite(BUZZER, LOW);
  } else if (suYuzdesi > 10) {
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(BUZZER, LOW);
  } else {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(BUZZER, HIGH);
  }
}
