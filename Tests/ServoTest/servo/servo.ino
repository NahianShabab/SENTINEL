#include<Servo.h>
int pos=0;
int direction=0;
Servo myServo;
unsigned long last=millis();
int moveDelay=15;
void setup() {
  // put your setup code here, to run once:
  myServo.attach(9);
  myServo.write(0);
  
}

void loop() {
  if(millis()-last>=15){
    last=millis();
    if(direction==0){
      myServo.write(++pos);
      if(pos==90){
        direction=1;
      }
    }else{
      myServo.write(--pos);
      if(pos==0){
        direction=0;
      }
    }
  }
}
