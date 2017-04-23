#include "packet.h"
#include <radio.h>

int pin = 3;
int MIN = 0;
int MAX = 255;

int RF_CHANNEL = 24;

void setup() {
  // put your setup code here, to run once:
  rfBegin(RF_CHANNEL);
  Serial.begin(9600);
  analogWrite(pin, 0);
}

int values[8];
char printBuf1[128];

void loop() {
  if(rfAvailable() >= 8){
    bool readPacket = receivePacket(values);

    if(readPacket){
      for(int i = 0; i < 8; ++i){
        sprintf(printBuf1, "%d ", values[i]);
        Serial.print(printBuf1); 
      }
      Serial.print('\n');
      setSpeed(values[0] >> 2);  
    }
    
  }
  
  delay(50);
}

void setSpeed(int s){
  if(s > MAX){
    s = MAX;
  }

  if(s < MIN){
    s = MIN;
  }

  analogWrite(pin, s);
}

