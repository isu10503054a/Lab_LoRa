#include "DHT.h"
#include <string.h>

#define DHTPIN A0
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

#define LED_PORT 4
#define FAN_PORT 5

char readcharbuffer[50];
int readbuffersize;
char temp_input;
int number = 1;

//Compare LoRa Response whether Data is we want
//mac rx 2 <data>, there is downlink data.
char rx_str[20] = ">> mac rx 2 ";

  
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
  
  Serial1.print("mac join abp");  //join mode of LoRa Gateway (use abp)
  Serial.println("mac join abp");

  readbuffersize = Serial1.available();
  while(readbuffersize){
    temp_input = Serial1.read();
    Serial.print(temp_input);
    readbuffersize--;
  }
    
  delay(3000);
  
}

int val_t,val_h;   //temperature & humidity int part
int val_tp,val_hp;  //temperature & humidity point part 

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


  //data for LoRa tx (temperature & humidity)
  //LoRa transmit data use a HEX string
  val_t = (int) t;
  val_tp=int ((t - val_t)*100);
  val_h = (int) h;
  val_hp=int ((h - val_h)*100);
  
  char lora_string[30];

  sprintf(lora_string, "mac tx ucnf 2 %02d%02d%02d%02d", val_t, val_tp, val_h, val_hp );
  Serial1.print(lora_string);  //tx now
  Serial.println(lora_string);  //tx now

  // received LoRa response and filter information
  unsigned long prev;
  int parse_ok = 0;
  // Initial call to setup the target string
  parseInput(rx_str,0);
  while(!parse_ok){
    int timeout = 0;
    prev = millis();
    while(Serial1.available() == 0){
      if(millis() - prev > 2500) {
        timeout = 1;
        break;
      }
    }
    if(timeout) break;
    temp_input = Serial1.read();
    Serial.print(temp_input);
    // call parseInput()
    if(parseInput(NULL , temp_input)) parse_ok = 1;
  }
  
  if(parse_ok){
    int i = 0;
    temp_input = 0;
    while(temp_input != '\n'){
      while(Serial1.available() == 0);
      temp_input = Serial1.read();
      Serial.print(temp_input);
      lora_string[i++] = temp_input;
    }
    lora_string[i-1] = '\0';
    
    //Control LED and FAN switch
    if((lora_string[0] - '0') | (lora_string[1] - '0')){
      //led_on
      Serial.println("led_on");
      digitalWrite(LED_PORT, HIGH);
     }
     else{
      //led_off
      Serial.println("led_off");
      digitalWrite(LED_PORT, LOW);
     }
     if((lora_string[2] - '0') | (lora_string[3] - '0')){
      //fan_on
      Serial.println("fan_on");
      digitalWrite(FAN_PORT, HIGH);
     }
     else{
      //fan_off
      Serial.println("fan_off");
      digitalWrite(FAN_PORT, LOW);
     }
   }
  else Serial.println("Parse timeout!");
  delay(2500);
   
}


#define MAX_TARGET_STRING_LENGTH  20
int parseInput(const char *target, const char input) {
  static int state, final;
  static char internal_str[MAX_TARGET_STRING_LENGTH+1];
  
  // Initial call: copy target string to internal str & 
  // set state = 0 and final = the length of target string;
  if (target!=NULL) {
    // If the length of target string is larger than the maximum allowed value,
    // return -1. 
    if (strlen(target) > MAX_TARGET_STRING_LENGTH)
      return(-1);
    //Copy target string to internal string
    strcpy(internal_str, target);
    // initialize state and final 
    state = 0;
    final = strlen(target);
    return 0;
  } 
  
  // Check if state has already been equal to final,
  // i.e. the last string match is finished and need to re-initialize,
  // then return -2
  if(state == final)
    return -2;
    
  // Compare input character with internal_str[state]:
  // if equal, state++
  // else, set state back to 0
  if(input == internal_str[state])
    state++;
  else
    state = 0;
  
  // If state is equal to final, 
  // i.e. the cumulative input characters match the internal string,
  // then reutrn 1;
  // else, return 0
  if(state==final)
    return 1;
  else
    return 0; 
}
