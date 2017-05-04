
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Simple_AHRS.h>

#include "packet.h"
#include "radio.h"

// Create LSM9DS0 board instance.
Adafruit_LSM9DS1     lsm(1000);  // Use I2C, ID #1000

int AFT_L_PIN = 3;
int AFT_R_PIN = 4;
int FT_L_PIN = 5;
int FT_R_PIN = 8;
int MIN = 0;
int MAX = 255;
int DEADZONE = 15;
float COMP_GAIN = .4;
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
Adafruit_Simple_AHRS ahrs(&lsm.getAccel(), &lsm.getMag(), &lsm.getGyro());

//orientation holder
sensors_vec_t orientation;

//PID VARS
float PID[3][3]; //0 - pitch, 1 roll, 2 yaw
float setpts[3]; //0 -pitch, 1 roll, 2 yaw
float sensorIns[3][2]; //0-pitch, 1 roll, 2 yaw
float PIDaccs[3];//accumulator for pitch, roll, yaw
long lastTimes[3];
float lastErr[3][20];
float outputs[3][12]; //0, pitch, 1 roll, 2 yaw holds memory of last 10 outputs for rolling filter


// Function to configure the sensors on the LSM9DS1 board.
// You don't need to change anything here, but have the option to select different
// range and gain values.
void configureLSM9DS1(void)
{
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
}

void initConstants(){
  PID[0][1] = 0;
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
  if(!lsm.begin())
  {
    // There was a problem detecting the LSM9DS0 ... check your connections
    Serial1.print(F("Ooops, no LSM9DS1 detected ... Check your wiring or I2C ADDR!"));
    while(1);
  }
  
  configureLSM9DS1();
  initConstants();
}

void loop() {
  // put your main code here, to run repeatedly:
  /*for( int i =0; i < MAX; i++){
    delay(1000);
    setSpeed(i);
  }*/
  /*if(rfAvailable() >= 8){
    receivePacket(arr);
    setSpeed(arr[0]);
    Serial.print(arr[0]);
  }*/

  readData();
  readSensors();
  pidCalc();
  if(packet.verify == 3435){
    writeToMotors();
  }
  //delay(1000);
  //setSpeed(MIN);
}

void readData(){
  if(rfAvailable() >= sizeof(Packet)){
    receivePacket(packet);
    //scale and add deadzone
    //change pitch from 0-1024 to -512 to 512
    packet.pitch = packet.pitch - 460;//value from the stick is offset
    if(packet.pitch < DEADZONE && packet.pitch > -DEADZONE){
      packet.pitch = 0;
    }
    //scale to be -45 to 45 degrees
    packet.pitch = ((float)packet.pitch) / 460.0 * 45.0;
    if(packet.pitch > 45){
      packet.pitch = 45;
    }
    //do the same for roll
    packet.roll = packet.roll - 460;
    if(packet.roll < DEADZONE && packet.roll > -DEADZONE){
      packet.roll = 0;
    }
    //scale to be -45 to 45 degrees
    packet.roll = ((float)packet.roll)/460.0*45.0;

    setpts[0] = packet.pitch;
    setpts[1] = packet.roll;
    setpts[2] = packet.yaw;
    //scale throttle
   packet.throttle = (((float)packet.throttle)/1024.0)*255.0;

   static bool enable = true;
   //change pid vals
   if(packet.btn2){
    enable = !enable;
   }
   //COMP_GAIN = ((float)packet.pot1)/1024.0;
   PID[0][0] = ((float)packet.pot1)/1024.0*3.0;
   if(enable){
   PID[0][2] = ((float)packet.pot2)/1024.0*3.0;
   }
   else{
    PID[0][1] = ((float)packet.pot2)/1024.0*3.0;
   }
   //PID[0][0] = 1.72;
   //PID[0][2] = 1.98;
   
    
  } 
}

void readSensors(){
  ahrs.getQuadOrientation(&orientation);
  sensorIns[0][0] = orientation.pitch;
  sensorIns[0][1] = orientation.pitch_rate/1000;//convert from deg/s to deg/ms
  sensorIns[1][0] = orientation.roll;
  sensorIns[1][1] = orientation.roll_rate/1000;
  sensorIns[2][0] = orientation.yaw;
  sensorIns[2][1] = orientation.yaw_rate/1000;
  /*Serial.print(sensorIns[0]);
  Serial.print(' ');
  Serial.print(sensorIns[1]);
  Serial.print(' ');
  Serial.print(sensorIns[2]);
  Serial.print(' ');*/
}
void pidCalc(){
  //loop for each of pitch, roll, yaw
  for (int i = 0; i < 3; i++){
    long currentTime = millis();
    long delta = currentTime-lastTimes[i];
    //complimentary filter
    sensorIns[i][0] = COMP_GAIN*(sensorIns[i][0] + delta*sensorIns[i][1]) + (1 - COMP_GAIN)*(sensorIns[i][0]);
    float error = setpts[i] - sensorIns[i][0];
    //avg out our error to smooth it
    for(int j = 0; j < 20; j++){
      error += lastErr[i][j];
    }
    error /=21.0;
    for(int j = 0; j < 19; j++){
      lastErr[i][j+1] = lastErr[i][j];
    }

    if(error < 1 && error > -1){
      error = 0;
    }
    
    PIDaccs[i] += error;
    //combat integral windup
    if(PIDaccs[i] > MAX/2){
      PIDaccs[i] = MAX/2;
    }
    else if(PIDaccs[i] < -MAX/2){
      PIDaccs[i] = -MAX/2;
    }
    float deltaError = error - lastErr[i][0];//change between now and last

    float output = PID[i][0]*error + PID[i][1]*PIDaccs[i]+PID[i][2]*deltaError;
    // we r gonna avg this output against all previous
    //two loops, one for the avg and one for the shiftin
    float avg = output;
    for(int j = 0; j<10; j++){
      avg+= outputs[i][j];
    }
    avg/= 11.0;
    for(int j = 0; j < 9; j++){
      outputs[i][j+1] = outputs[i][j];
    }
    if(avg >MAX/2){
      avg = MAX/2;
    }
    if(avg < -MAX/2){
      avg = - MAX/2;
    }
    outputs[i][0] = avg;
    lastErr[i][0] = error;
    lastTimes[i] = currentTime;
    Serial.print(error);
    Serial.print(' ');
    Serial.print(avg);
    Serial.print(' ');
    
    
  }
  Serial.print('\n');
  
}
void writeToMotors(){
  int throttle = packet.throttle;
  int aft_l;
  int aft_r;
  int ft_l;
  int ft_r;
  if(throttle > MAX){
    throttle = MAX;
  }

  if(throttle < MIN){
    throttle = MIN;
  }
  //
  if(throttle < DEADZONE){
    aft_r = MIN;
    aft_l = MIN;
    ft_l = MIN;
    ft_r = MIN;
  }
  else{

  aft_l = throttle + outputs[0][0] + outputs[1][0] - outputs[2][0];
  aft_r = throttle + outputs[0][0] - outputs[1][0] + outputs[2][0];
  ft_l = throttle - outputs[0][0] + outputs[1][0] + outputs[2][0];
  ft_r = throttle - outputs[0][0] - outputs[1][0] - outputs[2][0];
  }
  
  analogWrite(AFT_L_PIN, aft_l);
  analogWrite(AFT_R_PIN, aft_r);
  analogWrite(FT_L_PIN, ft_l);
  analogWrite(FT_R_PIN, ft_r);

}

