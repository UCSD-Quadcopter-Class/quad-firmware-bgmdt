#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM9DS0.h>
#include <Adafruit_Simple_AHRS.h>

#include "packet.h"
#include "radio.h"

// Create LSM9DS0 board instance.
Adafruit_LSM9DS0     lsm(1000);  // Use I2C, ID #1000

int AFT_L_PIN = 3;
int AFT_R_PIN = 4;
int FT_L_PIN = 5;
int FT_R_PIN = 8;
int MIN = 0;
int MAX = 255;
int arr[8];

/*typedef struct structPacket Packet;
struct structPacket{
  int throttle;
  int yaw;
  int pitch;
  int roll;
  int pot1;
  int pot2;
  int btn1;
  int btn2;
  int verify;
}*/

//packet to be recevied
Packet packet;
// Create simple AHRS algorithm using the LSM9DS0 instance's accelerometer and magnetometer.
Adafruit_Simple_AHRS ahrs(&lsm.getAccel(), &lsm.getMag());

// Function to configure the sensors on the LSM9DS0 board.
// You don't need to change anything here, but have the option to select different
// range and gain values.
void configureLSM9DS0(void)
{
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS0_ACCELRANGE_2G);
  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS0_MAGGAIN_2GAUSS);
  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS0_GYROSCALE_245DPS);
}

void setup() {
  // put your setup code here, to run once:
  analogWrite(AFT_L_PIN, 0);
  analogWrite(AFT_R_PIN, 0);
  analogWrite(FT_L_PIN, 0);
  analogWrite(FT_R_PIN, 0);


  rfBegin(24);
  Serial.begin(9600);  // Start up serial
  Serial1.begin(115200);
  //configureLSM9DS0();
}

void loop() {
  // put your main code here, to run repeatedly:
  /*for( int i =0; i < MAX; i++){
    delay(1000);
    setSpeed(i);
  }*/
  //delay(1000);
  /*if(rfAvailable() >= 8){
    receivePacket(arr);
    setSpeed(arr[0]);
    Serial.print(arr[0]);
  }*/

  readData();
  //readSensors();
  pidCalc();
  if(packet.verify == 3435){
    writeToMotors();
  }
  //delay(1000);
  //setSpeed(MIN);
  Serial.print(packet.throttle);
}

void readData(){
  if(rfAvailable() >= sizeof(Packet)){
    receivePacket(packet);
  } 
}

void readSensors(){
  
}
void pidCalc(){
  
}
void writeToMotors(){
  int s = packet.throttle;
  if(s > MAX){
    s = MAX;
  }

  if(s < MIN){
    s = MIN;
  }

  analogWrite(AFT_L_PIN, s);
  analogWrite(AFT_R_PIN, s);
  analogWrite(FT_L_PIN, s);
  analogWrite(FT_R_PIN, s);

}

