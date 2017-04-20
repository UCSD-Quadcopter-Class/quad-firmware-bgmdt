int pin = 3;
int MIN = 0;
int MAX = 255;
void setup() {
  // put your setup code here, to run once:
  analogWrite(pin, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  for( int i =0; i < MAX; i++){
    delay(1000);
    setSpeed(i);
  }
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

