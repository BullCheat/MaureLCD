/*! ===========================================================================================================================
  Projet: Afficheur LCD Putpitre
  Nom: DIVEL Clément
  Editor: Lycée Marcel Callo
  date: 29/01/2018
 */

#include <Arduino.h>
// include the library code:
#include <LiquidCrystal.h>

#define WIDTH 20

using namespace std;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void serialToOutput();
bool serialToCursor();
bool serialToLCD();
void updateButtons();
void updateThrottle();
byte readChannel(byte i);
byte readThrottle(byte i);
byte readResChar(byte i);

void setup() {
  lcd.begin(20, 4);
  Serial.begin(57600);
  pinMode(A15, INPUT);
}

void serialEvent() {
  static bool isText = false;
  while (Serial.available()) {

    if (isText) isText = serialToLCD();

    else if (Serial.read() == 0) {
      char action = Serial.read();
      if (action == 'T') isText = serialToCursor();
      else if (action == 'O') serialToOutput();
    }
  }
}

float lims[] = {-1 , -0.8, -0.6, -0.4, -0.2, 0,   0.2, 0.4, 0.6, 0.8, 1};

void loop() {
  updateButtons();
  // updateThrottle();
  Serial.println(lims[readResByte(A15)]);
}


void serialToOutput() {
  byte data = Serial.read();
  digitalWrite(data >> 1, data & 1);
}

bool serialToCursor() {
  int index = Serial.read();
  if (index >= 80) return false; // Hors limite
  lcd.setCursor(index % 20, index / 20);
  return true;
}

bool serialToLCD() {
  char c = Serial.read();
  if (c == -1)
    return true; // Pas de caractère lu, on retente
  if (c == 0)
    return false; // Caractère de stop : on sort de la fonction
  lcd.print(c);
  return true;
}

#define BUTTON_INDEX 37
#define BUTTON_COUNT 7
void updateButtons() {
  static byte statuses = 0;
  static long lasts[] = {0, 0, 0, 0, 0, 0, 0};
  for (byte i = 0; i < BUTTON_COUNT; i++) {
    bool old = (statuses >> i) & 1;
    bool nem = digitalRead(BUTTON_INDEX + i);
    if (old != nem && millis() - lasts[i] > 100) {
      lasts[i] = millis();
      statuses = statuses ^ (1 << i);
      if (nem) {
        Serial.print('\0');
        Serial.print('B');
        Serial.print(i);
      }
    }
  }
}

#define ANALOG_OFFSET 10

void updateThrottle() {
  static byte channel[] = {0,0};
  static byte value[] = {0,0};
  for (byte i = 0; i < 2; i++) {
    byte newChannel = readChannel(i);
    byte newThrottle = readThrottle(i);
    if (channel[i] != newChannel || value[i] != newThrottle) {
      Serial.print('\0');
      if (channel[i] != newChannel) {
        Serial.print(channel[i]);
        Serial.print(0);
      }
      Serial.print(newChannel);
      Serial.print(newThrottle);
      channel[i] = newChannel;
      value[i] = newThrottle;
    }
  }
}


byte readChannel(byte i) {
  return readResByte(A0 + ANALOG_OFFSET + i*3) + 1;
}
byte readThrottle(byte i) {
  byte resistance = readResChar(A0 + ANALOG_OFFSET + 1 + i*3);
  float limit = lims[resistance];
  int read = analogRead(A0 + ANALOG_OFFSET + 2 + i*3);
  return (byte) (0.125 * read * limit);
}


int seuils[] = {1000, 884,  800,  730,  650,  555, 455, 320, 210, 80 , 0}; // cf. excel

byte readResByte(byte i) {
  int read = analogRead(i);
  byte o = -1;
  while (true) {
    if (read >= seuils[++o]) {
      return o;
    }
  }
}
