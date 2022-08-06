void setup() {
  // put your setup code here, to run once:
  pinMode(5,INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  Serial.println(digitalRead(5));
}
