#include <FastLED.h>
#include <OneButton.h>
#include <SimpleTimer.h>
#include "IRremote.h"
#include <Arduino.h>

#define IR_RECEIVE_PIN 7

#define LED_PIN   6
#define NextShow_PIN   3  
#define NumCountUp_PIN   4


//-------Helper macro for getting a macro definition as string---------
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
//-----------------------



const uint8_t DisplayWidth = 14;
const uint8_t DisplayHeight = 10;

const uint16_t NumLeds = DisplayWidth * DisplayHeight;

CRGB leds[NumLeds];

const uint8_t MaxNum = 99;
uint8_t whichNum = 0;

const uint8_t MaxShow = 3;  //Change depending on how many different "shows"
uint8_t whichShow = 0;

OneButton NextShowBtn(NextShow_PIN, true, true);
OneButton NumCountUpBtn(NumCountUp_PIN, true, true);


//-----------Setting up font stuff-------------
const uint8_t FontWidth = 7;
const uint8_t FontHeight = 10;
const uint8_t NumCharacters = 10;

const uint8_t FontTable[NumCharacters][FontHeight] = {
	{ 0x7e, 0x7e, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7e, 0x7e },  // 0
	{ 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60 },  // 1
	{ 0x7e, 0x7e, 0x60, 0x60, 0x7e, 0x7e, 0x06, 0x06, 0x7e, 0x7e },  // 2
	{ 0x7e, 0x7e, 0x60, 0x60, 0x7c, 0x7c, 0x60, 0x60, 0x7e, 0x7e },  // 3
	{ 0x66, 0x66, 0x66, 0x66, 0x7e, 0x7e, 0x60, 0x60, 0x60, 0x60 },  // 4
	{ 0x7e, 0x7e, 0x06, 0x06, 0x7e, 0x7e, 0x60, 0x60, 0x7e, 0x7e },  // 5
	{ 0x06, 0x06, 0x06, 0x06, 0x7e, 0x7e, 0x66, 0x66, 0x7e, 0x7e },  // 6
	{ 0x7e, 0x7e, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60 },  // 7
	{ 0x7e, 0x7e, 0x66, 0x66, 0x7e, 0x7e, 0x66, 0x66, 0x7e, 0x7e },  // 8
	{ 0x7e, 0x7e, 0x66, 0x66, 0x7e, 0x7e, 0x60, 0x60, 0x60, 0x60 }   // 9
};

const uint8_t NumDigits = DisplayWidth / FontWidth;



//---------------------(START)Number Code/Counting and stuff--------------------
// Calculate the index for a given LED in the matrix, given its X/Y coordinates
uint16_t xy(uint16_t x, uint16_t y) {
	const int RowBase = DisplayWidth * y;  // number of LEDs before the current row
	if (x >= DisplayWidth) x = DisplayWidth - 1;
	if (y >= DisplayHeight) y = DisplayHeight - 1;

	uint16_t output;
	if (y % 2 == 0) output = RowBase + x;  // normal on even rows
	else output = RowBase + (DisplayWidth - x - 1);  // reversed on odd rows (serpentine)

	if (output >= NumLeds) output = NumLeds - 1;

	return output;
}

void clearDigit(int pos, CRGB color = CRGB::Black);
void writeDigit(int pos, int num, CRGB color, CRGB background = CRGB::Black);
void writeNumber(int num, CRGB color, CRGB background = CRGB::Black);


void clearDigit(int pos, CRGB color) {
	if (pos < 0 || pos >= NumDigits) return;  // display index out of range

	const uint8_t ColumnOffset = FontWidth * pos;  // offset the column position per digit

	for (uint8_t row = 0; row < FontHeight; row++) {
		for (uint8_t col = 0; col < FontWidth; col++) {
			leds[xy(col + ColumnOffset, row)] = color;  // assign color to LED array
		}
	}
}

void writeDigit(int pos, int num, CRGB color, CRGB background) {
	if (num < 0 || num >= NumCharacters) return;  // number out of range
	if (pos < 0 || pos >= NumDigits) return;  // display index out of range

	const uint8_t* Character = FontTable[num];  // get the font array from the table
	const uint8_t ColumnOffset = FontWidth * pos;  // offset the column position per digit

	for (uint8_t row = 0; row < FontHeight; row++) {
		for (uint8_t col = 0; col < FontWidth; col++) {
			const bool lit = Character[row] & (1 << col);  // extract bit for this LED
			leds[xy(col + ColumnOffset, row)] = lit ? color : background;  // assign color to LED array
		}
	}
}

void writeNumber(int num, CRGB color, CRGB background) {
	num = abs(num);  // not supporting negative numbers yet

	for (uint8_t i = 0; i < NumDigits; i++) {
		uint8_t digit = num % 10;
		writeDigit(NumDigits - i - 1, digit, color, background);  // right to left
		num /= 10;
	}
}
//---------------------(END)Number Code and stuff--------------------




//---------------------Show Numer on LEDs--------------------
void ShowNumber() {
	writeNumber(whichNum, CRGB::Blue);
	printLEDs();
}


//---------------------Button Counting--------------------
void IncrementNextShow() {
	whichShow++;
	if (whichShow >= MaxShow) whichShow = 0;
  ShowNumber();
	DebugStatus();
}

void IncrementNumberShow() {
	whichNum++;
	if (whichNum > MaxNum) whichNum = 0;
	DebugStatus();
	ShowNumber();
}



//---------------------SETUP--------------------
void setup() {
	FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NumLeds);
	FastLED.setBrightness(100);

	Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  
 //----------Debug IR-Remote Info----------------
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
  Serial.println(F("Enabling IRin..."));  
  Serial.print(F("Ready to receive IR signals of protocols: "));
  Serial.print(F("at pin "));
  Serial.println(IR_RECEIVE_PIN);

//----------Buttons----------
	NextShowBtn.attachClick(IncrementNextShow);
	NumCountUpBtn.attachClick(IncrementNumberShow);

	ShowNumber();

}

//---------IR Button Presses---------------
/*  --------Button List---------
"Button: Guide    Code: 0x1C"
"Button: Power    Code: 0x8"
"Button: TV       Code: 0xD6"
"Button: AV       Code: 0x51"
"Button: COMP     Code: 0x5A"
"Button: HDMI     Code: 0xC6"
"Button: Menu     Code: 0x43"
"Button: Up       Code: 0x45"
"Button: Down     Code: 0x46"
"Button: Left     Code: 0x47"
"Button: Right    Code: 0x48"
"Button: Mute     Code: 0x9"
"Button: Last     Code: 0x1A"
"Button: Vol-Up   Code: 0x2"
"Button: Vol-Down   Code: 0x3"
"Button: Ch-Up    Code: 0x0"
"Button: Ch-Down    Code: 0x1"
"Button: 1        Code: 0x11"
"Button: 2        Code: 0x12"
"Button: 3        Code: 0x13"
"Button: 4        Code: 0x14"
"Button: 5        Code: 0x15"
"Button: 6        Code: 0x16"
"Button: 7        Code: 0x17"
"Button: 8        Code: 0x18"
"Button: 9        Code: 0x19"
"Button: ?        Code: 0x2F"
"Button: 0        Code: 0x10"
"Button: ??       Code: 0xFF"
*/



//---------------------FakeDelay--------------------
void FakeDelay(int DelayTime){
  unsigned long time_now = millis();
  while(millis() < time_now + DelayTime){
  }
}




int incomingByte = 0; // for incoming serial data
//---------------------Loop--------------------
void loop() {

  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    if (incomingByte == 10){
      uint16_t RemoteIRNumber = 0x48;
      Serial.print("RemoteIRNumber: ");
      Serial.println(RemoteIRNumber, HEX);
    }
    Serial.print("incomingByte: ");
    Serial.println(incomingByte, DEC);
  }
   incomingByte == 0;
   
	//NextShowBtn.tick();
	//NumCountUpBtn.tick();
  if (IrReceiver.decode()){
  IrReceiver.printIRResultShort(&Serial);
  //IrReceiver.printIRResultMinimal(&Serial);
  uint16_t RemoteIRNumber = IrReceiver.decodedIRData.command;
  Serial.println(RemoteIRNumber, HEX);
  
  if (RemoteIRNumber == 0x48){
    Serial.print(RemoteIRNumber);
    Serial.print("BlinkShow");
    BlinkShow();
    }
  if (RemoteIRNumber == 0x47){
    Serial.print(RemoteIRNumber);
    Serial.print("Flagshow");
    FlagShow();
    }
  IrReceiver.resume();
  }

	/*switch (whichShow) {
	case 0:
		break;  // LEDs written in button function
	case 1:
		FlagShow();
		break;
	case 2:
		BlinkShow();
		break;
	}*/
	FastLED.show();
}




//---------------------FlagShow--------------------
void FlagShow() {
	leds[0] = CRGB(0, 0, 255);
	leds[1] = CRGB(0, 0, 255);
	leds[2] = CRGB(0, 0, 255);
	leds[3] = CRGB(255, 0, 0);
	leds[4] = CRGB(255, 0, 0);
	leds[5] = CRGB(255, 0, 0);
	leds[6] = CRGB(255, 0, 0);
	leds[7] = CRGB(255, 255, 255);
	leds[8] = CRGB(255, 255, 255);
	leds[9] = CRGB(255, 255, 255);
	leds[10] = CRGB(255, 255, 255);
	leds[11] = CRGB(0, 0, 255);
	leds[12] = CRGB(255, 255, 255);
	leds[13] = CRGB(0, 0, 255);
	leds[14] = CRGB(0, 0, 255);
	leds[15] = CRGB(0, 0, 255);
	leds[16] = CRGB(0, 0, 255);
	leds[17] = CRGB(255, 0, 0);
	leds[18] = CRGB(255, 0, 0);
	leds[19] = CRGB(255, 0, 0);
	leds[20] = CRGB(255, 0, 0);
	leds[21] = CRGB(255, 255, 255);
	leds[22] = CRGB(255, 255, 255);
	leds[23] = CRGB(255, 255, 255);
	leds[24] = CRGB(255, 255, 255);
	leds[25] = CRGB(255, 255, 255);
	leds[26] = CRGB(255, 255, 255);
	leds[27] = CRGB(255, 255, 255);
	leds[28] = CRGB(255, 0, 0);
	leds[29] = CRGB(255, 0, 0);
	leds[30] = CRGB(255, 0, 0);
	leds[31] = CRGB(255, 0, 0);
	leds[32] = CRGB(255, 0, 0);
	leds[33] = CRGB(255, 0, 0);
	leds[34] = CRGB(255, 0, 0);
  FastLED.show();
}

//---------------------Blink--------------------
void BlinkShow() {
		FastLED.clear();
		fill_solid(leds, NumLeds, CRGB::Blue);
		FastLED.show();
    FakeDelay(1000);
    FastLED.clear();
    fill_solid(leds, NumLeds, CRGB::Red);
    FastLED.show();
    FakeDelay(3000);
}


//---------------------Debugging Code--------------------

//-----------SerialOut for Button Pushes
void DebugStatus() {
	Serial.print("ShowNumber: ");
	Serial.print(whichShow);
	Serial.print(" WhichNumber: ");
	Serial.print(whichNum);
	Serial.print('\n');
}

//-----------Debug visualization for the LED on/off states
void printLEDs() {
  for (uint8_t row = 0; row < DisplayHeight; row++) {
    for (uint8_t col = 0; col < DisplayWidth; col++) {
      bool on = leds[xy(col, row)].getLuma() >= 16;  // arbitrary threshold
      Serial.print(on ? 'X' : '_');
      Serial.print(" ");
    }
    Serial.println();
  }
}
