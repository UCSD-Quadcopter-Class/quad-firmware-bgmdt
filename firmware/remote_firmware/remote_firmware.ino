#include <radio.h>
#include "packet.h"

/* test_remote.ino
 * Sketch for testing all parts of QuadRemote.
 * Components tested:
 *    Gimbals: 
 *    Indicator LEDs
 *    Buttons
 *    Serial display
 *    Potentiometers
 *    Radio communications
 */

#include "quad_remote.h"      // Header file with pin definitions and setup
#include <serLCD.h>

// Configuration and Setup Flags
bool configuredFlag = false;

// Initialize global variables for storing incoming data from input pins
int readYaw = 0;
int readThrottle = 0;
int readRoll = 0;
int readPitch = 0; 
int readPot1 = 0;
int readPot2 = 0;
int button1Value = 0;     // buttons are active high
int button2Value = 0; 
bool button1Press = 0;
bool button2Press = 0;
bool LEDVal = 0;

uint8_t scale[8] = 
                 {B00000000,
                  B00000000,
                  B00000000,
                  B00000000,
                  B00000000,
                  B00000000,
                  B00000000,
                  B00000000};

int maxVals[8];
int minVals[8];
float scaleVals[8];

int numbers[9] = {0,1,2,3,4,5,6,7, 3435};
char *labels[8] = {"T ", "Y ", "P ", "R ", "P1", "P2", "B1", "B2"};

char pins[8] = {PIN_THROTTLE, PIN_YAW, PIN_PITCH, PIN_ROLL, PIN_POT1, PIN_POT2, PIN_POT1, PIN_POT2};


serLCD lcd;

void update_display() {
  lcd.clear();
  lcd.home();

  for(char h = 0; h < 8; h++) {
      char buf[2];
      buf[0] = labels[h][0];
      buf[1] = 0;
      lcd.print(buf);
      int n = numbers[h] >> 6;
      if (n > 8) {
         lcd.printCustomChar(n - 8);
      } else {
         lcd.print(" ");
      }
  }
  //lcd.print("\r");

  for(char h = 0; h < 8; h++) {
      char buf[2];
      buf[0] = labels[h][1];
      buf[1] = 0;
      lcd.print(buf);
      int n = numbers[h] >> 6;
      if (n >= 8) {
         lcd.printCustomChar(8);
      } else if (n == 0) {
        lcd.print(" ");
      } else {
         lcd.printCustomChar(n);
      }
  }
}

void setup() {
 
  const int RADIO_CHANNEL = 24;        // Channel for radio communications (can be 11-26)
  const int SERIAL_BAUD = 9600;        // Baud rate for serial port 
  const int SERIAL1_BAUD = 9600;     // Baud rate for serial1 port

  Serial.begin(SERIAL_BAUD);           // Start up serial
  Serial1.begin(SERIAL_BAUD);  
  
  delay(100);
  for(char i = 0; i < 8; i++) {
    scale[7-i] = B11111111;
    lcd.createChar(i+1, scale);
    delay(10);
  }
 
  rfBegin(RADIO_CHANNEL);              // Initialize ATmega128RFA1 radio on given channel
  
  pinMode(PIN_YAW, INPUT);             // Gimbal: Yaw
  pinMode(PIN_THROTTLE, INPUT);        // Gimbal: throttle
  pinMode(PIN_ROLL, INPUT);            // Gimbal: roll
  pinMode(PIN_PITCH, INPUT);           // Gimbal: pitch
  pinMode(PIN_POT1, INPUT);            // Potentiometer 1
  pinMode(PIN_POT2, INPUT);            // Potentiometer 2
  
  pinMode(PIN_BTN1, INPUT_PULLUP);            // Button 1
  pinMode(PIN_BTN2, INPUT_PULLUP);            // Button 2
  
  pinMode(PIN_LED_BLUE, OUTPUT);       // LED Indicator: Blue
  pinMode(PIN_LED_GRN, OUTPUT);        // LED Indicator: Green
  pinMode(PIN_LED_RED, OUTPUT);        // LED Indicator: Red

}

unsigned long lastRead = 0;
void loop() {

  /* Remote Control Main Loop */

  // Read incoming presses from buttons
  numbers[6] = digitalRead(PIN_BTN1) ? 0: 1024; 
  numbers[7] = digitalRead(PIN_BTN2)? 0: 1024; 
    
  if(configuredFlag){
  
    if (lastRead + 1000 <= millis()) {
      LEDVal = 1;//!LEDVal;
      digitalWrite(PIN_LED_BLUE, LEDVal);
      digitalWrite(PIN_LED_GRN, LEDVal);
      digitalWrite(PIN_LED_RED, LEDVal);
      lastRead = millis();
    }
    
    // Read analog values
    for(char i = 0; i < 6; i++) {
      numbers[i] = analogRead(pins[i]); 
    }

    // Scale analog value based on min/maxes
    for(char i = 0; i < 6; ++i){
      numbers[i] = (int)((numbers[i] - minVals[i]) * scaleVals[i]);
      numbers[i] = numbers[i] < 0 ? 0 : numbers[i];
      numbers[i] = numbers[i] > 1023 ? 1023 : numbers[i];
    }

    // Update display with graph of values
    update_display();

    sendPacket((uint8_t*)&numbers, 18);
    
  } else {
    // Not configured
    
    lcd.clear();
    lcd.home();
    lcd.print("Configure:        Use Button 1");
    delay(500);
    
    if(numbers[6]){
      configurationMode();
    }
  }

  delay(100);

}

void printConfigurationCountdown(char * strMsg, int seconds){
  char printBuf[128];
  for(int i = seconds; i > 0; --i){
    sprintf(printBuf, "%s   0%ds", strMsg, i);
    lcd.clear();
    lcd.home();
    lcd.print(printBuf);
    delay(1000);
  }
  
  lcd.clear();
  lcd.home();
  lcd.print("Reading...");
  delay(200);
}

void configurationMode(){
  printConfigurationCountdown("MAX:", 3);

  for(int i = 0; i < 4; ++i){
    maxVals[i] = analogRead(pins[i]);
  }
  maxVals[6] = 1024;
  maxVals[7] = 1024;

  printConfigurationCountdown("MIN:", 3);

  for(int i = 0; i < 4; ++i){
    minVals[i] = analogRead(pins[i]);
  }
  minVals[6] = 0;
  minVals[7] = 0;

  printConfigurationCountdown("MAX:", 9);

  for(int i = 4; i < 6; ++i){
    maxVals[i] = analogRead(pins[i]);
  }
  
  printConfigurationCountdown("MIN:", 9);

  for(int i = 4; i < 6; ++i){
    minVals[i] = analogRead(pins[i]);
  }

  for(int i = 0; i < 6; ++i){
    scaleVals[i] = float(1023) / (maxVals[i] - minVals[i]);
  }
  
  scale[6] = 1;
  scale[7] = 1;

  lcd.clear();
  lcd.home();
  lcd.print("Configured...");
  
  configuredFlag = true;
}
