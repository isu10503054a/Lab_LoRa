#include <LWiFi.h>
#include <WiFiClient.h>
#include "MCS.h"
#include "DHT.h"

#define DHTPIN A0
#define DHTTYPE DHT22 
#define LED_PIN 7

// Assign AP ssid / password here
#define _SSID "3715"
#define _KEY  "12345678"

// Assign device id / key of your test device
MCSDevice mcs("DySDuWA0", "GWGPiejKT6hTmn1Y");


MCSDisplayFloat temperature("temperature");
MCSDisplayFloat humidity("humidity");
MCSControllerOnOff led("led");

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // setup Serial output at 9600
  Serial.begin(9600);

  dht.begin();
  // setup Wifi connection
  while(WL_CONNECTED != WiFi.status())
  {
    Serial.print("WiFi.begin(");
    Serial.print(_SSID);
    Serial.print(",");
    Serial.print(_KEY);
    Serial.println(")...");
    WiFi.begin(_SSID, _KEY);
  }
  Serial.println("WiFi connected !!");

  // setup MCS connection
  mcs.addChannel(temperature);
  mcs.addChannel(humidity);
  mcs.addChannel(led);
  
  // setup LED 
  pinMode(LED_BUILTIN, OUTPUT);
  
  while(!mcs.connected())
  {
    Serial.println("MCS.connect()...");
    mcs.connect();
  }
  Serial.println("MCS connected !!");

  // read LED value from MCS server

}

void loop() {
  // call process() to allow background processing, add timeout to avoid high cpu usage
  Serial.print("process(");
  Serial.print(millis());
  Serial.println(")");
  mcs.process(100);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  humidity.set(h);
  temperature.set(t);
  Serial.print("Humidity: "); 
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(t);
  Serial.println(" *C");
        
  if(led.updated())
    {
      Serial.print("LED updated, new value = ");
      Serial.println(led.value());
      digitalWrite(LED_PIN, led.value() ? HIGH : LOW);
    }
    
  // check if need to re-connect
  while(!mcs.connected())
  {
    Serial.println("re-connect to MCS...");
    mcs.connect();
    if(mcs.connected())
      Serial.println("MCS connected !!");
  }
}
