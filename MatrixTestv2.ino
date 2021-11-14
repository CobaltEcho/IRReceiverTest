#include <FastLED.h>
#include <OneButton.h>

#define NUM_LEDS  35
#define LED_PIN   6
#define NextShow_PIN   3  
#define NumCountUp_PIN   4

CRGB leds[NUM_LEDS];

uint8_t whichNum = 0;
uint8_t whichShow = 0;

OneButton NextShowBtn(NextShow_PIN, true, true);
OneButton NumCountUpBtn(NumCountUp_PIN, true, true);

void checkTick() {
    NextShowBtn.tick();
    NumCountUpBtn.tick();
}

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);

  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(NextShow_PIN),checkTick,CHANGE);
  attachInterrupt(digitalPinToInterrupt(NumCountUp_PIN),checkTick,CHANGE);
  NextShowBtn.attachClick(IncrementNextShow);
  NumCountUpBtn.attachClick(IncrementNumberShow);
}

void CurrentStatus(){
  Serial.print("ShowNumber: ");
  Serial.print(whichShow);
  Serial.print(" WhichNumber: ");
  Serial.print(whichNum);
  Serial.print('\n');
}

void loop() {
  NextShowBtn.tick();
  NumCountUpBtn.tick();
  FastLED.show();

  while (whichShow == 2) {
    NextShowBtn.tick();
    BlinkShow();
}
}

void IncrementNextShow() {
  whichShow = (whichShow + 1) % 3;
  CurrentStatus();
  DisplayNextShow();
}

void DisplayNextShow() {
  switch (whichShow){
    case 0:
    DisplayNumberShow();
    break;
    case 1:
    FlagShow();
    break;
    case 2:
//    BlinkShow();
    break;
  }
  FastLED.show();
}

void IncrementNumberShow() {
whichNum = (whichNum + 1) % 7;
CurrentStatus();
DisplayNumberShow();
}

void DisplayNumberShow() {
  switch (whichNum) {
    case 0:
      Num0();
      break;
    case 1:
      Num1();
      break;
    case 2:
      Num2();
      break;
    case 3:
      Num3();
      break;
    case 4:
      Num4();
      break;
    case 5:
      Num5();
      break;
    case 6:
      Num6();
      break;
   }
     FastLED.show();

}
