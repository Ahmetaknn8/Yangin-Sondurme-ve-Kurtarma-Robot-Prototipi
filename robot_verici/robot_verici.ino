#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
const byte adres[] = "00001";

// Joystick pinleri
const byte joyX = A0;
const byte joyY = A1;

// Potansiyometre pinleri
const byte potPin[5] = {A2, A3, A4, A5, A6};

// Buton pinleri
const byte ileriButon = 2;
const byte geriButon  = 3;
const byte solButon   = 4;
const byte sagButon   = 5;
const byte toggleSwitchPin  = 6; // Toggle switch (INPUT_PULLUP)

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setDataRate(RF24_250KBPS);  // Veri hızını 250 kbps yap
  radio.openWritingPipe(adres);
  radio.stopListening();

  pinMode(ileriButon, INPUT_PULLUP);
  pinMode(geriButon,  INPUT_PULLUP);
  pinMode(solButon,   INPUT_PULLUP);
  pinMode(sagButon,   INPUT_PULLUP);
  pinMode(toggleSwitchPin,  INPUT_PULLUP);
}

void loop() {
  byte veri[13];

  // Joystick verileri
  veri[0] = map(analogRead(joyX), 0, 1023, 0, 255);
  veri[1] = map(analogRead(joyY), 0, 1023, 0, 255);

  // 5 potansiyometre verisi
  for (int i = 0; i < 5; i++) {
    veri[2 + i] = map(analogRead(potPin[i]), 0, 1023, 0, 255);
  }

  // Butonlar (LOW = basılı)
  veri[7] = digitalRead(ileriButon) == LOW ? 1 : 0;
  veri[8] = digitalRead(geriButon)  == LOW ? 1 : 0;
  veri[9] = digitalRead(solButon)   == LOW ? 1 : 0;
  veri[10] = digitalRead(sagButon)  == LOW ? 1 : 0;
  veri[11] = digitalRead(toggleSwitchPin) == LOW ? 1 : 0; // Toggle switch

  // Rezerve alan (gelecekte kullanım için)
  veri[12] = 0;

  radio.write(&veri, sizeof(veri));
  delay(50);
}