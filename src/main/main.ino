#include <LiquidCrystal.h>

 /*pin assign*/
#define WINDOW_SERVO_PIN 5
#define LDR_PIN A0
#define DHT_PIN A1
#define GAS_PIN A2
#define BUZZER_PIN 10
#define LIGHT_PIN 8
#define AUTO_LIGHT_PIN 9
#define WATER_HIGH_PIN 3
#define WATER_LOW_PIN 4
#define RAIN_PIN 13
#define FAN_PIN 7
#define ECHO_PIN 11 /*ultrasonic sensor pins*/   
#define TRIGGER_PIN 12 /*ultrasonic sensor pins*/
#define PUMP_PIN 2


/*Sensitivity */
  /*ldr*/
#define LDR_SENSITIVITY 600 // if analog reading at LDR_PIN is greater than this value, AUTO_LIGHT Will be ON 
  /*Intruder/Ultrasonic sensor*/
#define INTRUDER_COOLDOWN_DURATION 1500 //in miliseconds, sensor will not catch another motion untill this time is passed
#define INTRUDER_DISTANCE_THRESHOLD 0.8 /* if distance<normalDistance*threshold, then motion detected */

/*Other settings*/
#define SENSOR_READ_FREQUENCY 50 // time between successive reads, miliseconds

/* window servo rotational speed in every 10 ms */
#define WIN_SERVO_SPEED 3
#define WIN_OPEN_ANGLE 110
#define WIN_CLOSE_ANGLE 8
#include<Servo.h>
#include<DHT.h>
#include<LiquidCrystal_I2C.h>
#define DHT_TYPE DHT11

/*global variables */
Servo winServo;
unsigned long sensorLastRead;
DHT dht(DHT_PIN,DHT_TYPE);
float temp,humidity;
byte enableAutoLight;
byte waterHigh,waterLow;
bool raining;
bool closeWindowRain;
bool intruderDetected;
bool detectIntruderEnabled;
unsigned long intruderLastDetected;
float normalDistance; /*ultrasonic device's calculated distance at setup()*/
LiquidCrystal_I2C lcd(0x27,16,2);

enum WinServoDir{OPEN,CLOSE};
WinServoDir winServoDir;
unsigned long winServoLastMoved;
bool gasDetected;
bool pumpOn;

void setup() {
  /*window servo */
  winServo.attach(WINDOW_SERVO_PIN);
  winServoDir=CLOSE;
//  winServo.write(WIN_CLOSE_ANGLE);
  winServoLastMoved=millis();

  /*Serial communication initialize*/
  Serial.begin(9600);

  /*set sensor last Read*/
  sensorLastRead=millis();

  /*dht setup*/
  dht.begin();
  temp=-500,humidity=-1;

  /*lcd setup*/
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print((char)223); // degree symbol
  lcd.setCursor(6,0);
  lcd.print('C');
  lcd.setCursor(8,0);
  lcd.print("H=");
  lcd.setCursor(15,0);
  lcd.print('%');
  lcd.setCursor(0,1);
  lcd.print("Jar=0%  ");

  /*Buzzer*/
  pinMode(BUZZER_PIN,OUTPUT);
  digitalWrite(BUZZER_PIN,LOW);

  /* Light Pin */
  pinMode(LIGHT_PIN,OUTPUT);
  digitalWrite(LIGHT_PIN,LOW);
  
  /*Auto Light Pin*/
  pinMode(AUTO_LIGHT_PIN,OUTPUT);
  digitalWrite(AUTO_LIGHT_PIN,LOW);
  enableAutoLight=1;

  /*water level*/
  pinMode(WATER_HIGH_PIN,INPUT);
  pinMode(WATER_LOW_PIN,INPUT);
  waterHigh=waterLow=LOW;
  pumpOn=false;

  /*Rain Sensor*/
  pinMode(RAIN_PIN,INPUT);
  raining=false;
  closeWindowRain=true;

  /*Fan*/
  pinMode(FAN_PIN,OUTPUT);
  digitalWrite(FAN_PIN,LOW);

  /*ultrasonic sensor*/
  pinMode(ECHO_PIN,INPUT);
  pinMode(TRIGGER_PIN,OUTPUT);
  intruderLastDetected=millis();
  intruderDetected=false;
  detectIntruderEnabled=true;
  normalDistance=0;
  for(int i=1;i<=10;i++){
    normalDistance+=getDistance();
  }
  normalDistance/=10; /*average distance*/
  Serial.print("normal distance: ");
  Serial.println(normalDistance);

  /*gas */
  pinMode(GAS_PIN,INPUT);
  gasDetected=false;

  /*pump*/
  pinMode(PUMP_PIN,OUTPUT);
  digitalWrite(PUMP_PIN,LOW);

}
//
//void openWindow(){
//  winServo.write(0);
//}
//
//void closeWindow(){
//  winServo.write(90);
//}

void moveWinServo(){
  //move only 100 ms interval
  if(millis()-winServoLastMoved>=10){
    if(winServoDir==OPEN){
      if(winServo.read()+WIN_SERVO_SPEED<=WIN_OPEN_ANGLE){
        winServo.write(winServo.read()+WIN_SERVO_SPEED);
        winServoLastMoved=millis();
      }else if(winServo.read()<WIN_OPEN_ANGLE){
        winServo.write(WIN_OPEN_ANGLE);
        winServoLastMoved=millis();
      }
    }
    else{
      if(winServo.read()-WIN_SERVO_SPEED>=WIN_CLOSE_ANGLE){
        winServo.write(winServo.read()-WIN_SERVO_SPEED);
        winServoLastMoved=millis();
      }else if(winServo.read()>WIN_CLOSE_ANGLE){
        winServo.write(WIN_CLOSE_ANGLE);
        winServoLastMoved=millis();
      }
    }
  }
}

void loop() {
    updateSensorValues(); // updates in Macro SENSOR_READ_FREQUENCY miliseconds interval
    
     /* water needs to be checked separately from other sensors all the time, since pump 
    may overflow*/
    updateWaterLevel();

    detectIntruder();

    moveWinServo();
    
    handleBluetoothRequest();

    detectGas();
}


/*helper functions*/

void updateSensorValues(){
   if(millis()-sensorLastRead>SENSOR_READ_FREQUENCY){

     /*ldr */
     float ldrValue=analogRead(LDR_PIN);
//     Serial.println(ldrValue);
     int oldAutoLight=digitalRead(AUTO_LIGHT_PIN);
     if(ldrValue>LDR_SENSITIVITY && enableAutoLight==1){
       digitalWrite(AUTO_LIGHT_PIN,HIGH);
       if(oldAutoLight==LOW)
           Serial.println("autolight_status 1");
     }else{
       digitalWrite(AUTO_LIGHT_PIN,LOW);
       if(oldAutoLight==HIGH)
           Serial.println("autolight_status 0");
     }


     /*temperature and humidity*/
     temp=dht.readTemperature();
//     Serial.println(temp);
     humidity=dht.readHumidity();
     /*update lcd with latest temp and humidity*/
     /*Note: it can be optimized by updating only when temperature/humid is changed*/
     lcd.setCursor(0,0);
     lcd.print(temp);
     lcd.setCursor(10,0);
     lcd.print(humidity);

     /*rain sensor*/
     if(digitalRead(RAIN_PIN)==LOW){
        if(closeWindowRain==true && gasDetected==false){
          winServoDir=CLOSE;
        }
        if(raining==false){
          if(intruderDetected==false && gasDetected==false){
            lcd.setCursor(8,1);
            lcd.print("Raining!");
          }
          raining=true;
          /*Write rain status to bluetooth*/
          Serial.println("rain_status 1");
        }
     }else{
        if(raining==true){
          
          if(intruderDetected==false && gasDetected==false){
            lcd.setCursor(8,1);
            lcd.print("        ");
          }
          raining=false;
           /*Write rain status to bluetooth*/
          Serial.println("rain_status 0");
        }
     }
     
     
     /* sensor read finished, reset last read to current time*/
     sensorLastRead=millis();
  }
}

void updateWaterLevel(){
    /*temporary water level variables*/
    byte wHigh=!digitalRead(WATER_HIGH_PIN);
    byte wLow=!digitalRead(WATER_LOW_PIN);
    if(wHigh==HIGH){
      if(waterHigh==LOW){
           lcd.setCursor(0,1);
           lcd.print("Jar=100%");
           Serial.println("tank_status 100%");
      }
      digitalWrite(PUMP_PIN,LOW);
      pumpOn=false;
    }else{
      if(wLow==HIGH){
        if(waterLow==LOW || waterHigh==HIGH){
          lcd.setCursor(0,1);
          lcd.print("Jar<100%");
          Serial.println("tank_status <100%");
          if(!pumpOn){
            digitalWrite(PUMP_PIN,LOW);
          }
        }
      }else{
        if(waterLow==HIGH){
          lcd.setCursor(0,1);
          lcd.print("Jar=0%  ");
          Serial.println("tank_status 0%");
        }
        digitalWrite(PUMP_PIN,HIGH);
        pumpOn=true;
      }
    }
    waterHigh=wHigh;
    waterLow=wLow;
}


/*get distance from ultrasonic sensor*/
/*NOTE: can optimize by using accurate values of velocity of sound using temp sensor*/
float getDistance(){
   // Clears the trigPin condition
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(ECHO_PIN, HIGH);
  // Calculating the distance
  float distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  return distance;
}

void detectGas(){
  if(digitalRead(GAS_PIN)==LOW){
    if(gasDetected==false){
      digitalWrite(BUZZER_PIN,HIGH);
      lcd.setCursor(8,1);
      lcd.print("  Gas!  ");

      Serial.println("gas_detected 1");
    }
    gasDetected=true;
    winServoDir=OPEN;
  }else{
    if(gasDetected==true){
      if(intruderDetected==true){
        lcd.setCursor(8,1);
        lcd.print("Intruder");
      }else if(raining==true){
          digitalWrite(BUZZER_PIN,LOW);
          lcd.setCursor(8,1);
          lcd.print("Raining!");
      }else{
         digitalWrite(BUZZER_PIN,LOW);
         lcd.setCursor(8,1);
         lcd.print("        ");
      }
    }
    gasDetected=false;
  }
}

void detectIntruder(){
    if((millis()-intruderLastDetected)>INTRUDER_COOLDOWN_DURATION && intruderDetected==true){
      intruderDetected=false;
      if(gasDetected==false){
        digitalWrite(BUZZER_PIN,LOW);
        if(raining==true){
          lcd.setCursor(8,1);
          lcd.print("Raining!");
        }else{
          lcd.setCursor(8,1);
          lcd.print("        ");
        }
      }
    }
    if(intruderDetected==false && detectIntruderEnabled==true){
//      Serial.println("here");
      float distance=getDistance();
//      Serial.print("disrance is: ");
//      Serial.println(distance);
      if(distance<=normalDistance*INTRUDER_DISTANCE_THRESHOLD){
        intruderDetected=true;
        intruderLastDetected=millis();
        Serial.println("intruder_detected 1");
        digitalWrite(BUZZER_PIN,HIGH);
        if(gasDetected==false){
          lcd.setCursor(8,1);
          lcd.print("Intruder");
        }
      }
    }
}


void handleBluetoothRequest(){
  /*Serial Receive*/
  if(Serial.available()>0){
    String cmd=Serial.readStringUntil('\n');
    if(cmd=="light 1"){
      digitalWrite(LIGHT_PIN,HIGH);
    }else if(cmd=="light 0"){
      digitalWrite(LIGHT_PIN,LOW);
    }
    if(cmd=="light_status"){
      if(digitalRead(LIGHT_PIN)==HIGH){
        Serial.println("light_status 1");
      }else{
        Serial.println("light_status 0");
      }
    }
//    else if(cmd=="window 1"){
//      openWindow();
//    }else if(cmd=="window 0"){
//      closeWindow();
//    }
    else if(cmd=="close_window_rain 0"){
        closeWindowRain=false;
    }
    else if(cmd=="close_window_rain 1"){
        closeWindowRain=true;
    }
    else if(cmd=="close_window_rain_status"){
      if(closeWindowRain==true){
        Serial.println("close_window_rain_status 1");
      }else{
        Serial.println("close_window_rain_status 0");
      }
    }
    else if(cmd=="close_window" && gasDetected==false){
      winServoDir=CLOSE;
    }
    else if(cmd=="open_window" && (raining==false || closeWindowRain==false)){
      winServoDir=OPEN;
    }   
    else if(cmd=="temp"){
      Serial.println("temp "+String(temp));
    }else if(cmd=="humidity"){
      Serial.println("humidity "+String(humidity));
    }
    else if(cmd=="autolight_status"){
      if(digitalRead(AUTO_LIGHT_PIN)==HIGH){
        Serial.println("autolight_status 1");
      }else{
        Serial.println("autolight_status 0");
      }
    }else if(cmd=="autolight_enable"){
      enableAutoLight=1;
    }else if(cmd=="autolight_disable"){
      enableAutoLight=0;
    }
    else if(cmd=="autolight_disable_status"){
      if(enableAutoLight==1){
        Serial.println("autolight_disable_status 0");
      }else
         Serial.println("autolight_disable_status 1");
    }
    /*user cannot manually toggle buzzer*/
//    else if(cmd=="buzzer_enable"){
//      digitalWrite(BUZZER_PIN,HIGH);
//    }else if(cmd=="buzzer_disable"){
//      digitalWrite(BUZZER_PIN,LOW);
//    }
    else if(cmd=="tank_status"){
      String status;
      if(waterHigh==HIGH){
        status="100%";
      }else if(waterLow==HIGH){
        status="<100%";
      }else{
        status="0%";
      }
      Serial.println("tank_status "+status);
    }else if(cmd=="rain_status"){
      if(raining==false){
        Serial.println("rain_status 0");
      }else{
        Serial.println("rain_status 1");
      }
    }else if(cmd=="fan 0"){
      digitalWrite(FAN_PIN,LOW);
    }else if(cmd=="fan 1"){
      digitalWrite(FAN_PIN,HIGH);
    }else if(cmd=="fan_status"){
      if(digitalRead(FAN_PIN)==HIGH){
        Serial.println("fan_status 1");
      }else{
         Serial.println("fan_status 0");
      }
    }else if(cmd=="detect_intruder_status"){
      if(detectIntruderEnabled==true){
        Serial.println("detect_intruder_status 1");
      }else
        Serial.println("detect_intruder_status 0");
    }else if(cmd=="detect_intruder 1"){
      detectIntruderEnabled=true;
    }else if(cmd=="detect_intruder 0"){
      detectIntruderEnabled=false;
    }else if(cmd=="pump_status"){
      if(digitalRead(PUMP_PIN)==HIGH){
        Serial.println("pump_status 1");
      }else
        Serial.println("pump_status 0");
    }
  }
}
