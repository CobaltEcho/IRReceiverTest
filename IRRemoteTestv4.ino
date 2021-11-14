#include "IRremote.h"
#include <Arduino.h>
#define IR_RECEIVE_PIN 7

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
  Serial.println(F("Enabling IRin..."));
  Serial.print(F("Ready to receive IR signals of protocols: "));
  //printActiveIRProtocols(&Serial);
  Serial.print(F("at pin "));
  Serial.println(IR_RECEIVE_PIN);
}


void loop() {
    if (IrReceiver.decode()){
    //IrReceiver.printIRResultShort(&Serial);
    //IrReceiver.printIRResultMinimal(&Serial);
    uint16_t RemoteIRNumber = IrReceiver.decodedIRData.command;
    Serial.println(RemoteIRNumber, HEX);
    
    if (RemoteIRNumber == 0x45){
      SaySomething();
    }
    if (RemoteIRNumber == 0x46){
      SaySomething2();
    }
    IrReceiver.resume();
    }
  }

void SaySomething(){
  Serial.print('\n');
  Serial.print("Say Something");
  Serial.print('\n');
}
void SaySomething2(){
  Serial.print('\n');
  Serial.print("Say Something Again");
  Serial.print('\n');
}
