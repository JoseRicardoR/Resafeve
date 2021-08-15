void setup() {
  Serial.begin(115200);
  //analogSetCycles(4);
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
  int analog = analogRead(34);
  Serial.print("0, 4095, 2048, ");
  Serial.println(analog);
  }
