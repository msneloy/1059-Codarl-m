#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>               
#include <TimeLib.h>  
#include <LedControl.h>
#include "WAC.h"

LedControl lc=LedControl(D0,D8,D4,1); //DIN,CLK,CS,display

byte last_second, second_, minute_, hour_, day_;  
int Digit7,Digit6,Digit5,Digit4,Digit3,Digit2,Digit1,Digit0;

const char *ssid = SSID;
const char *password = PW;
//Create WAC.h to store creds
//#define SSID "Wireless Access Point"
//#define PW "Password"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 28800, 60000);

void setup() {
  
  Serial.begin(115200);
  
  lc.shutdown(0,false);
  lc.setIntensity(0,15);
  lc.clearDisplay(0);

  WiFi.begin(ssid, password);
  Serial.print("Connecting.");

  while ( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();

}

void loop() { 

  timeClient.update();
  unsigned long unix_epoch = timeClient.getEpochTime();
    second_ = second(unix_epoch);
  if (last_second != second_) {
 

    minute_ = minute(unix_epoch);
    hour_   = hour(unix_epoch);
    day_    = day(unix_epoch);

    Digit0 = second_ % 10;
    Digit1 = second_ / 10;
    Digit2  = minute_ % 10;
    Digit3  = minute_ / 10;
    Digit4  = hour_   % 10;
    Digit5  = hour_   / 10;
    Digit6  = day_   %  10;
    Digit7  = day_   /10;
    
  Serial.println(day_);
  Serial.println(hour_);
  Serial.println(minute_);
  Serial.println(second_);
  
  last_second = second_;

  lc.setDigit(0,7,(byte)Digit7,false);
  lc.setDigit(0,6,(byte)Digit6,true);
  lc.setDigit(0,5,(byte)Digit5,false);
  lc.setDigit(0,4,(byte)Digit4,true);
  lc.setDigit(0,3,(byte)Digit3,false);
  lc.setDigit(0,2,(byte)Digit2,true);
  lc.setDigit(0,1,(byte)Digit1,false);
  lc.setDigit(0,0,(byte)Digit0,false);
     
  }

}
