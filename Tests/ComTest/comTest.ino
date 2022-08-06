
byte LED1=11;
byte LED2=12;
byte DarkLED=13;
byte LDRPin=A0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(DarkLED,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    String cmd=Serial.readStringUntil('\n');
    if(cmd=="LED1"){
      digitalWrite(LED1,!digitalRead(LED1)); 
    }else if(cmd=="LED2"){
      digitalWrite(LED2,!digitalRead(LED2));
    }else if(cmd=="S1"){
        int s1Value=analogRead(LDRPin);
        String respnse("S1 ");
        respnse.concat(s1Value);
        respnse.concat("\n");
        Serial.write(respnse.c_str());
    }else if(cmd=="S2"){
      long s2Value=random(10,100);
      String respnse("S2 ");
        respnse.concat(s2Value);
        respnse.concat("\n");
        Serial.write(respnse.c_str());
    }
  }
  if(analogRead(LDRPin)>500){
    digitalWrite(DarkLED,HIGH);
  }else{
    digitalWrite(DarkLED,LOW);
  }
}
