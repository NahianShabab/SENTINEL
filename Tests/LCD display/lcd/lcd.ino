#include<Wire.h>
#include<LiquidCrystal_I2C.h>


//initialize the liquid crystal library
//the first parameter is the I2C address
//the second parameter is how many rows are on your screen
//the third parameter is how many columns are on your screen
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  
  //initialize lcd screen
  // turn on the backlight
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("abc");
  
}
void loop() {
}
