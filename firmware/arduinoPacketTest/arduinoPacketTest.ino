#include "packet.h"

unsigned int numbers[8];
void setup() {
  // put your setup code here, to run once:
  numbers[0] = 0x03ff;
  numbers[1] = 0x03fe;
  numbers[2] = 0x03fd;
  numbers[3] = 0x03fc;
  numbers[4] = 0x03fb;
  numbers[5] = 0x03fa;
  numbers[6] = 1024;
  numbers[7] = 1024;

  Serial.begin(9600);
}

int results[8];

char printBuf[128];
void loop() {
  for(int i = 0; i < 8; ++i){
    results[i] = 0;
  }
  
  // put your main code here, to run repeatedly:
  char * packet = sendPacket(numbers);
  
  Serial.print("Checking Packet...\n");
  
  for(int i = 0; i < 8; ++i){
    sprintf(printBuf, "%u: %x\n", i, packet[i] & B11111111);
    Serial.print(printBuf);
  }
  Serial.print('\n');
  delay(500);
  
  receivePacket(results, packet);

  Serial.print("Beginning Loop:\n");
  for(int i = 0; i <= 7; ++i){
    sprintf(printBuf, "%u: %x %x\n", i, numbers[i], results[i]);
    Serial.print(printBuf);
    delay(200);
  }

  

  free(packet);
}
