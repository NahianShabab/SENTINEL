#include"testLibrary.h"
sample s(2,3);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(s.x);
  Serial.println(s.y);
  delay(1000);
}
