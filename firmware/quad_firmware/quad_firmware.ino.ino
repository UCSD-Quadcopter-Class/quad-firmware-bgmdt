int pin = 3;
int MIN = 0;
int MAX = 255;
int arr[8];

#include "packet.h"
#include "radio.h"

void setup() {
  // put your setup code here, to run once:
  analogWrite(pin, 0);
  rfBegin(24);
  Serial.begin(9600);  // Start up serial
  Serial1.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  /*for( int i =0; i < MAX; i++){
    delay(1000);
    setSpeed(i);
  }*/
  //delay(1000);
  if(rfAvailable() >= 8){
    receivePacket(arr);
    setSpeed(arr[0]);
    Serial.print(arr[0]);
  }
  //delay(1000);
  //setSpeed(MIN);
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

