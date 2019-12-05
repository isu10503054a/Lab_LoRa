/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 2  (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)


 Note:
 Not all pins on LinkIt 7697 support change interrupts,
 so only the following can be used for RX:
 2, 3

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 modified Dec 2017
 by MediaTek Labs

 This example code is in the public domain.

 */
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SeeedOLED.h>

// On LinkIt 7697, the RX pin must be one of the EINT pins, such as P2 and P3.
// There are no limitations on TX pin.
SoftwareSerial mySerial(2, 11); // RX(P2), TX

void setup() {
  // OLED initialize
  Wire.begin();  
  SeeedOled.init();  //initialze SEEED OLED display
  SeeedOled.clearDisplay();           //clear the screen and set start position to top left corner
  SeeedOled.setNormalDisplay();       //Set display to Normal mode
  SeeedOled.setPageMode();            //Set addressing mode to Page Mode
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Ublox Neo 7M");
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);

  Serial1.begin(115200);
  // set the data rate for the SoftwareSerial port
  pinMode(12, OUTPUT);  //for rest EK-S76SXB
  digitalWrite(12, LOW);
  delay(100);
  pinMode(12, INPUT);  //for rest EK-S76SXB  
  delay(2000);
  
  Serial1.print("mac join abp");  //join mode of LoRaWAN (use abp)
  Serial.println("mac join abp");
  delay(1000);
  
}

char msg[200];
char head[7];
char utc_str[15];
char lat_str[15];
char ns_str[2];
char lon_str[15];
char ew_str[2];
int msg_length = 0;
char lora_string[50];
int serial_no = 0;


void loop() { // run over and over
  int p, index, i, j;
  if(serial_no >= 99999){serial_no = 0;}
  if (mySerial.available()) {
    msg[msg_length] = mySerial.read();
    Serial.write(msg[msg_length]);
    msg_length++;
    // if a complete line is received, i.e. read a '\n', Parse the line according NMEA format
    if(msg[msg_length-1]=='\n') {
      msg[msg_length] = '\0';
      // Get the first token (6 characters) of this line
      strncpy(head, msg, 6);
      head[6] = '\0';
      //Serial.println(head);
      // if first token is "$GNGGA", get the position data and output it to OLED screen
      if(!strcmp(head,"$GNGGA")) {
        // get UTC string
        p = 7;  index = 0;
        while(msg[p]!=',') {
          utc_str[index] = msg[p];
          index++; p++;
        }
        utc_str[index]='\0';
        if(utc_str[0]=='\0') strcpy(utc_str, "<No UTC Data>");
        //Serial.println(utc_str);
        
        //get Latitude string
        p++; index = 0;
        while(msg[p]!=',') {
          lat_str[index] = msg[p];
          index++; p++;
        }
        lat_str[index]='\0';
        if(lat_str[0]=='\0') strcpy(lat_str, "<No Lat. Data>");
        
        p++; index = 0;
        while(msg[p]!=',') {
          ns_str[index] = msg[p];
          index++; p++;
        }
        ns_str[index]='\0';
        if(ns_str[0]!='\0') strcat(lat_str, ns_str);
        //Serial.println(lat_str);

        //get Longitude string
        p++; index = 0;
        while(msg[p]!=',') {
          lon_str[index] = msg[p];
          index++; p++;
        }
        lon_str[index]='\0';
        if(lon_str[0]=='\0') strcpy(lon_str, "<No Lon. Data>");
        
        p++; index = 0;
        while(msg[p]!=',') {
          ew_str[index] = msg[p];
          index++; p++;
        }
        ew_str[index]='\0';
        if(ew_str[0]!='\0') strcat(lon_str, ew_str);
        //Serial.println(lon_str);

        // output data to OLED
        //SeeedOled.clearDisplay();           //clear the screen and set start position to top left corner
        SeeedOled.setTextXY(0,0);           //Set the cursor to 0th Page, 0th Column  
        //SeeedOled.putString(utc_str);       //Print UTC string
        SeeedOled.putNumber(serial_no);       //Print serial number
        SeeedOled.setTextXY(1,0);           //Set the cursor to 1st Page, 0th Column  
        SeeedOled.putString(lat_str);       //Print Lat. string
        SeeedOled.setTextXY(2,0);           //Set the cursor to 2nd Page, 0th Column  
        SeeedOled.putString(lon_str);       //Print Lon. string
        Serial.print("Time: ");
        Serial.print(utc_str);
        Serial.print(", Lat: ");
        Serial.print(lat_str);
        Serial.print(", Lon: ");
        Serial.println(lon_str);
        
        Serial.println("====="); 
        del_string(lat_str);
        del_string(lon_str);
        int lat =atoi(lat_str);
        int lon =atoi(lon_str);
        sprintf(lora_string, "mac tx ucnf 2 %05d%8d%9d",  serial_no, lat, lon);
        Serial1.print(lora_string);  //tx now
        Serial.println(lora_string);  //tx now
        serial_no++;
        delay(3500);
      }
      msg_length = 0;
    }
  }
}

void del_string (char s[10]){
    int j ,k;
    for(j=k=0; s[j]!='\0'; j++)
      if(s[j]!='.')
        s[k++]=s[j];
      s[k]= '\0';
    return ;
  }
