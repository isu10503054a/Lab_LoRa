String lora_string; //string for TX

void setup() {
  Serial.begin(9600);
  Serial1.begin(115200);

  // set the data rate for the SoftwareSerial port
  pinMode(12, OUTPUT);  //for rest EK-S76SXB
  digitalWrite(12, LOW);
  delay(100);
  pinMode(12, INPUT);  //for rest EK-S76SXB  
  delay(1500);
  
  Serial1.print("mac join abp");  //join mode of LoRaWAN (use abp)
  Serial.println("mac join abp");
  delay(1000);
  
  lora_string="mac tx ucnf 2 1234567890"; //for tx header
  Serial1.print(lora_string);  //tx now
  Serial.println(lora_string);  //tx now

  
}

void loop() {

}
