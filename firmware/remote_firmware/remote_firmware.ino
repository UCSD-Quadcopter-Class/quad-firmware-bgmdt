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

int numbers[8] = {0,1,2,3,4,5,6,7};
char *labels[8] = {"T ", "Y ", "P ", "R ", "P1", "P2", "B1", "B2"};
char pins[8] = {PIN_THROTTLE, PIN_YAW, PIN_PITCH, PIN_ROLL, PIN_POT1, PIN_POT2, PIN_POT1, PIN_POT2};
bool configuredFlag = false;

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

  //lcd.print("Hello, World!");
 
  const int RADIO_CHANNEL = 25;        // Channel for radio communications (can be 11-26)
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
  
  // Send a message to other RF boards on this channel
  //rfPrint("ATmega128RFA1 Dev Board Online!\r\n");
  
  // Set pin modes for all input pins
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

int last = 0;

void loop() {

  /* BUTTON TEST: Print to serial when button press registered */

  // Read incoming presses from buttons: WHY AREN'T INTERRUPTS WORKING
  button1Value = digitalRead(PIN_BTN1); 
  button2Value = digitalRead(PIN_BTN2); 
    
  // Print to serial if press registered
  numbers[6] = button1Value? 0 : 1024;
  numbers[7] = button2Value? 0 : 1024;



  if(configuredFlag){
  
    if (last + 1000 <= millis()) {
      LEDVal = 1;//!LEDVal;
      digitalWrite(PIN_LED_BLUE, LEDVal);
      digitalWrite(PIN_LED_GRN, LEDVal);
      digitalWrite(PIN_LED_RED, LEDVal);
      last = millis();
    }
    // Read analog values
    for(char i = 0; i < 6; i++) {
      numbers[i] = analogRead(pins[i]); 
    }

    for(char i = 0; i < 6; ++i){
      numbers[i] = (int)((numbers[i] - minVals[i]) * scaleVals[i]);
      numbers[i] = numbers[i] < 0 ? 0 : numbers[i];
      numbers[i] = numbers[i] > 1023 ? 1023 : numbers[i];
    }
  
    update_display();
    
    sendPacket(numbers);
  } else {
    lcd.clear();
    lcd.home();
    lcd.print("Configure:        Use Button 1");
    delay(500);
    if(numbers[6]){
      enterConfigurationMode();
    }
  }
 
  delay(100);

}

void printMaxConfigurationCountdown(){
  char printBuf[128];
  for(int i = 3; i > 0; --i){
    sprintf(printBuf, "MAX:   0%ds      ", i);
    lcd.clear();
    lcd.home();
    lcd.print(printBuf);
    delay(1000);
  }
  lcd.clear();
  lcd.home();
  lcd.print("Reading YTRP...");
  delay(200);
}

void printMinConfigurationCountdown(){
  char printBuf[128];
  for(int i = 3; i > 0; --i){
    sprintf(printBuf, "MIN:   0%ds      ", i);
    lcd.clear();
    lcd.home();
    lcd.print(printBuf);
    delay(1000);
  }
  lcd.clear();
  lcd.home();
  lcd.print("Reading YTRP...");
  delay(200);  
}

void printMaxPotCountdown(){
  char printBuf[128];
  for(int i = 9; i > 0; --i){
    sprintf(printBuf, "MAX:   0%ds      ", i);
    lcd.clear();
    lcd.home();
    lcd.print(printBuf);
    delay(1000);
  }
  lcd.clear();
  lcd.home();
  lcd.print("Reading POTs...");
  delay(1000);  
}

void printMinPotCountdown(){
  char printBuf[128];
  for(int i = 9; i > 0; --i){
    sprintf(printBuf, "MIN:   0%ds      ", i);
    lcd.clear();
    lcd.home();
    lcd.print(printBuf);
    delay(1000);
  }
  lcd.clear();
  lcd.home();
  lcd.print("Reading POTs...");
  delay(1000);  
}

void enterConfigurationMode(){
  printMaxConfigurationCountdown();

  for(int i = 0; i < 4; ++i){
    maxVals[i] = analogRead(pins[i]);
  }
  maxVals[6] = 1024;
  maxVals[7] = 1024;

  printMinConfigurationCountdown();

  for(int i = 0; i < 4; ++i){
    minVals[i] = analogRead(pins[i]);
  }
  minVals[6] = 0;
  minVals[7] = 0;

  printMaxPotCountdown();

  for(int i = 4; i < 6; ++i){
    maxVals[i] = analogRead(pins[i]);
  }
  
  printMinPotCountdown();

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
