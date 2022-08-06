/*pin assign*/
#define WINDOW_SERVO_PIN 5
#define LDR_PIN A0
#define DHT_PIN 11
#define BUZZER_PIN 12
#define LIGHT_PIN 7
#define AUTO_LIGHT_PIN  6
#define WATER_HIGH_PIN 3
#define WATER_LOW_PIN 4
#define RAIN_PIN 13

/*Sensitivity */
#define LDR_SENSITIVITY 450 // if analog reading at LDR_PIN is greater than this value, AUTO_LIGHT Will be ON 

/*Other settings*/
#define SENSOR_READ_FREQUENCY 50 // time between successive reads, miliseconds


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
LiquidCrystal_I2C lcd(0x27,16,2);


void setup() {
  /*window servo */
  winServo.attach(WINDOW_SERVO_PIN);
  closeWindow();

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
  lcd.print("Tank=0%  ");

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

  /*Rain Sensor*/
  pinMode(RAIN_PIN,INPUT);
  raining=false;

}

void openWindow(){
  winServo.write(0);
}

void closeWindow(){
  winServo.write(90);
}

void loop() {
    updateSensorValues(); // updates in Macro SENSOR_READ_FREQUENCY miliseconds interval
    
     /* water needs to be checked separately from other sensors, since pump 
    may overflow*/
    updateWaterLevel();

    handleBluetoothRequest();
}


/*helper functions*/

void updateSensorValues(){
   if(millis()-sensorLastRead>SENSOR_READ_FREQUENCY){

     /*ldr */
     float ldrValue=analogRead(LDR_PIN);
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
        if(raining==false){
          lcd.setCursor(8,1);
          lcd.print("Raining!");
          raining=true;

          /*Write rain status to bluetooth*/
          Serial.println("rain_status 1");
        }
     }else{
        if(raining==true){
          lcd.setCursor(8,1);
          lcd.print("        ");
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
           lcd.print("Tank=100%");
           Serial.println("tank_status 100%");
      }
    }else{
      if(wLow==HIGH){
        if(waterLow==LOW || waterHigh==HIGH){
          lcd.setCursor(0,1);
          lcd.print("Tank<100%");
          Serial.println("tank_status <100%");
        }
      }else{
        if(waterLow==HIGH){
          lcd.setCursor(0,1);
          lcd.print("Tank=0%  ");
          Serial.println("tank_status 0%");
        }
      }
    }
    waterHigh=wHigh;
    waterLow=wLow;
}

void handleBluetoothRequest(){
  /*Serial Receive*/
  if(Serial.available()>0){
    String cmd=Serial.readStringUntil('\n');
    if(cmd=="light 1"){
      digitalWrite(LIGHT_PIN,HIGH);
    }else if(cmd=="light 0"){
      digitalWrite(LIGHT_PIN,LOW);
    }else if(cmd=="window 1"){
      openWindow();
    }else if(cmd=="window 0"){
      closeWindow();
    }else if(cmd=="temp"){
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
    }else if(cmd=="buzzer_enable"){
      digitalWrite(BUZZER_PIN,HIGH);
    }else if(cmd=="buzzer_disable"){
      digitalWrite(BUZZER_PIN,LOW);
    }else if(cmd=="tank_status"){
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
    }
  }
}
