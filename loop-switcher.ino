#include<EasyButton.h>
#include<EEPROM.h>

#define latchPin 10
#define clockPin 12
#define dataPin 11
#define modeLED A0
#define bankLED 13
int relayPin[8] = {2, 3, 4, 5, 6, 7, 8, 9};

#define ampBoostPin 1 // TX
#define ampChannelPin A7
int presses = 2;
int timeout = 200;

byte preset = B00000000;
byte loops = B00000000;

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;

int i = 0;
int buttonFlag = -5;
unsigned int pressed = 0;
unsigned int mode = 0;
unsigned int bank = 0;
unsigned int isBoost = 0;
int lastPressed = -5;

EasyButton BTN0(A1);
EasyButton BTN1(A2);
EasyButton BTN2(A3);
EasyButton BTN3(A4);
EasyButton BTN4(A5);


void toggleBoost(int led){
  isBoost = !isBoost;
  digitalWrite(ampBoostPin, !isBoost);
  flashPresetLED(led);
  bitWrite(preset, led, 1);
  shiftBits();
}

void boostOff(){
  isBoost = 0;
  digitalWrite(ampBoostPin, HIGH);
}

void shiftBits() {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, preset);
  shiftOut(dataPin, clockPin, LSBFIRST, loops);
  digitalWrite(latchPin, HIGH);
  delay(100);
}

void writeLoops(int num) {
  bitWrite(loops, num, !bitRead(loops, num));  
  digitalWrite(relayPin[num], !bitRead(loops, num));
  shiftBits();
}

void pullAllLow() {
  loops = B00000000;
  preset = B00000000;
  shiftBits();
  for (i = 0; i < 8; i++)
    digitalWrite(relayPin[i], HIGH);
  digitalWrite(ampBoostPin, HIGH);
  boostOff();
}

void flashPresetLED(int led) {
  bitWrite(preset, led, 1);
  shiftBits();
  bitWrite(preset, led, 0);
  shiftBits();
  bitWrite(preset, led, 1);
  shiftBits();
  bitWrite(preset, led, 0);
  shiftBits();
}

void changeBank() {
  boostOff();
  bank++;
  digitalWrite(bankLED, !digitalRead(bankLED));
  for (i = 0; i < 8; i++) {
    if (bitRead(preset, i) == 1) {
      //if (bank % 2 == 0)
        readPreset((bank % 2 * 100) + 1 + 10 * i, i);
      //else if (bank % 2 == 1)
        //readPreset(101 + 10 * i, i);
    }
  }
}

void changeMode() {
  mode++;
  digitalWrite(modeLED, !digitalRead(modeLED));
  pullAllLow();
  lastPressed = -5;
}

bool calculateTime() {
  previousMillis = currentMillis;
  currentMillis = millis();
  if (currentMillis - previousMillis >= 50)
    return true;
  return false;
}

void memorizePreset(int address, int led) {
  for (i = 0; i < 8; i++)
    EEPROM.write((address) + i, bitRead(loops, i));
  flashPresetLED(led);
  pullAllLow();
}

void readPreset(int address, int led) {
  for (i = 0; i < 8; i++) {
    digitalWrite(relayPin[i], !EEPROM.read((address) + i));
    bitWrite(loops, i, EEPROM.read((address) + i));
  }
  preset = B00000000;
  bitWrite(preset, led, 1);
  shiftBits();
}

void startup(){
  for(i = 8; i >= 0; i--){
    bitWrite(loops, i, 1);
    bitWrite(preset, i, 1);
    shiftBits();
    bitWrite(loops, 1+i, 0);
    bitWrite(preset, 1+i, 0);
  }
  digitalWrite(modeLED, HIGH);
  digitalWrite(bankLED, HIGH);
  pullAllLow();
  digitalWrite(modeLED, LOW);
  digitalWrite(bankLED, LOW);
}


//SWITCHING-----------------------------------------------------

int findPressedButton() {
  if (buttonFlag == 4) {
    if (BTN4.isReleased() && BTN3.isPressed())
      return 6;
    else if (BTN4.isReleased())
      return 7;
  }
  if (buttonFlag == 3) {
    if (BTN3.isReleased() && BTN2.isPressed())
      return 4;
    else if (BTN3.isReleased() && BTN4.isPressed())
      return 6;
    else if (BTN3.isReleased())
      return 5;
  }
  if (buttonFlag == 2) {
    if (BTN2.isReleased() && BTN1.isPressed())
      return 2;
    else if (BTN2.isReleased() && BTN3.isPressed())
      return 4;
    else if (BTN2.isReleased())
      return 3;
  }
  if (buttonFlag == 1) {
    if (BTN1.isReleased() && BTN0.isPressed())
      return 0;
    else if (BTN1.isReleased() && BTN2.isPressed())
      return 2;
    else if (BTN1.isReleased())
      return 1;
  }
  if (buttonFlag == 0) {
    if (BTN0.isReleased() && BTN1.isPressed())
      return 0;
    else
      return -1;
  }
}

void BTN4_Pressed() {
  buttonFlag = 4;
  Pressed();
}
void BTN3_Pressed() {
  buttonFlag = 3;
  Pressed();
}
void BTN2_Pressed() {
  buttonFlag = 2;
  Pressed();
}
void BTN1_Pressed() {
  buttonFlag = 1;
  Pressed();
}
void BTN0_Pressed() {
  buttonFlag = 0;
  Pressed();
}

void Pressed() {
  if (calculateTime()) {
    int number = findPressedButton();
    if (mode % 2 == 0) { //LOOP MODE
      preset = B00000000;
      if (number == -1)
        changeBank();
      else
        writeLoops(number);
    }
    else {  // PRESET MODE
      if (number == -1)
        changeBank();
      else{
          readPreset(10 * number + (bank % 2 * 100) + 1, number);
          if (lastPressed == number)
            toggleBoost(lastPressed);
          else
            boostOff();
          lastPressed = number;
      }       
    }
  }
  currentMillis = millis();
  buttonFlag = -2;
}


int findButtonHeld() {
  if (buttonFlag == 4) {
    if (BTN3.isPressed())
      return 6;
    else if (!BTN3.isPressed())
      return 7;
    }
  if (buttonFlag == 3) {
    if (BTN2.isPressed())
      return 4;
    else if (BTN4.isPressed())
      return 6;
    else if (!BTN2.isPressed() && !BTN4.isPressed())
      return 5;
  }
  if (buttonFlag == 2) {
    if (BTN1.isPressed())
      return 2;
    else if (BTN3.isPressed())
      return 4;
    else if (!BTN1.isPressed() && !BTN3.isPressed())
      return 3;
  }
  if (buttonFlag == 1) {
    if (BTN0.isPressed())
      return 0;
    else if (BTN2.isPressed())
      return 2;
    else if (!BTN0.isPressed() && !BTN2.isPressed())
      return 1;
  }
  if (buttonFlag == 0) {
    if (BTN1.isPressed())
      return 0;
    else
      return -1;
  }
}

void BTN0_Held() {
  buttonFlag = 0;
  Held();
}
void BTN1_Held() {
  buttonFlag = 1;
  Held();
}
void BTN2_Held() {
  buttonFlag = 2;
  Held();
}
void BTN3_Held() {
  buttonFlag = 3;
  Held();
}
void BTN4_Held() {
  buttonFlag = 4;
  Held();
}

void Held() {
  int number = findButtonHeld();
  if(calculateTime()){
  if (number != -1 && mode % 2 == 0) {
    if (bank % 2 == 0)
      memorizePreset(10 * number + 1, number);
    else if (bank % 2 == 1)
      memorizePreset(10 * number + 101, number);
  }
  else
    if (number == -1)
      changeMode();
  }
  currentMillis = millis();
}


void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(modeLED, OUTPUT);
  pinMode(bankLED, OUTPUT);
  pinMode(ampBoostPin, OUTPUT);
  for (i = 0; i < 8; i++){
    pinMode(relayPin[i], OUTPUT);
    digitalWrite(relayPin[i], HIGH);
  }
  pullAllLow();
  
  BTN0.onPressed(BTN0_Pressed);
  BTN0.onPressedFor(1000, BTN0_Held);
  BTN1.onPressed(BTN1_Pressed);
  BTN1.onPressedFor(1000, BTN1_Held);
  BTN2.onPressed(BTN2_Pressed);
  BTN2.onPressedFor(1000, BTN2_Held);
  BTN3.onPressed(BTN3_Pressed);
  BTN3.onPressedFor(1000, BTN3_Held);
  BTN4.onPressed(BTN4_Pressed);
  BTN4.onPressedFor(1000, BTN4_Held);
  
  //BTN0.onSequence(presses, timeout, toggleBoost);
  
  BTN0.begin();
  BTN1.begin();
  BTN2.begin();
  BTN3.begin();
  BTN4.begin();

  startup();
}

void loop() {
  BTN0.read();
  BTN1.read();
  BTN2.read();
  BTN3.read();
  BTN4.read();
}
