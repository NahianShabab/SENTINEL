#include"SevenSegment.h"
void setup() {
  for(int i=2;i<=9;i++)
    pinMode(i,OUTPUT);
  // put your setup code here, to run once:
}

unsigned char characters[]={
  0b11111100, // 0
  0b01100000, // 1
  0b11011010, // 2
  0b11110010, // 3
  0b01100110, // 4
  0b10110110, // 5
  0b10111110, // 6
  0b11100000, // 7
  0b11111110, // 8
  0b11110110,  // 9
  0b11111010, // a
  0b00111110, // b
  0b10011100, // c
  0b01111010, // d
  0b11011110, // e
  0b10001110  // f
};
int count=0;
void loop() {
  // put your main code here, to run repeatedly:
  printCharacter(characters[count]);
  count=(count+1)%16;
  delay(1000);
}
