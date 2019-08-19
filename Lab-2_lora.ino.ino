#include "DHT.h"

#define DHTPIN A0
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial1.begin(115200);

  dht.begin();
  Serial.println("DHTxx test!");

  // set the data rate for the SoftwareSerial port
  pinMode(12, OUTPUT);  //for rest EK-S76SXB
  digitalWrite(12, LOW);
  delay(100);
  pinMode(12, INPUT);  //for rest EK-S76SXB  
  delay(2000);
  
  Serial1.print("mac join abp");  //join mode of LoRaWAN (use abp)
  Serial.println("mac join abp");
  delay(5000);
  
}

int val_t,val_h;   //temperature & humidity int part
int val_tp,val_hp;  //temperature & humidity point part 

String lora_string; //string for TX

void loop() {

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C\t ");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");


////////////next for LoRa tx /////////////////////////////
  
  val_t = (int) t;
  val_tp=int ((t - val_t)*100);
  val_h = (int) h;
  val_hp=int ((h - val_h)*100);
  
  lora_string="mac tx ucnf 2 "; //for tx header

  //temperature & humidity for tx (hex format)
  if (val_t<10) lora_string+="0";
  lora_string+=val_t;
  if (val_tp<10) lora_string+="0";
  lora_string+=val_tp;
  if (val_h<10) lora_string+="0";
  lora_string+=val_h;
  if (val_hp<10) lora_string+="0";
  lora_string+=val_hp;
  Serial1.print(lora_string);  //tx now
  Serial.println(lora_string);  //tx now

  
  delay(3500);
}
