p32 void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
  Serial.println("Ready");
}

void loop() {
  // put your main code here, to run repeatedly:
 float sensorValue1 = analogRead(32);
 Serial.println(sensorValue1);
 delay(300);
}
