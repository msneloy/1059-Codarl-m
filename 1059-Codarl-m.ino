#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h> //https://codeload.github.com/johnrickman/LiquidCrystal_I2C/zip/master
#include <WiFiUdp.h>
#include <NTPClient.h>               
#include <TimeLib.h>  
#include <LedControl.h>
#include <ESPAsyncWebServer.h>
#include "WAC.h"

LedControl lc=LedControl(D0,D3,D4,1); //DIN,CLK,CS,display
LiquidCrystal_I2C lcd(0x27, 16, 2);  
byte last_second, second_, minute_, hour_, day_;  
int Digit7,Digit6,Digit5,Digit4,Digit3,Digit2,Digit1,Digit0;

const char *ssid = SSID;
const char *password = PW;
//Create WAC.h to store creds
//#define SSID "Wireless Access Point"
//#define PW "Password"
#define RELAY_NO    true
// Set number of relays
#define NUM_RELAYS  4
// Assign each GPIO to a relay
int relayGPIOs[NUM_RELAYS] = {D5, D6, D7, D8};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 28800, 60000);
const char* PARAM_INPUT_1 = "relay";  
const char* PARAM_INPUT_2 = "state";
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    body {background: firebrick;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: red; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: green}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>Test Bench 01</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?relay="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?relay="+element.id+"&state=0", true); }
  xhr.send();
}</script>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    for(int i=1; i<=NUM_RELAYS; i++){
      String relayStateValue = relayState(i);
      buttons+= "<h4>Relay #" + String(i) + " - GPIO " + relayGPIOs[i-1] + "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" + String(i) + "\" "+ relayStateValue +"><span class=\"slider\"></span></label>";
    }
    return buttons;
  }
  return String();
}

String relayState(int numRelay){
  if(RELAY_NO){
    if(digitalRead(relayGPIOs[numRelay-1])){
      return "";
    }
    else {
      return "checked";
    }
  }
  else {
    if(digitalRead(relayGPIOs[numRelay-1])){
      return "checked";
    }
    else {
      return "";
    }
  }
  return "";
}
void setup() {
  
  Serial.begin(115200);
  for(int i=1; i<=NUM_RELAYS; i++){
    pinMode(relayGPIOs[i-1], OUTPUT);
    if(RELAY_NO){
      digitalWrite(relayGPIOs[i-1], HIGH);
    }
    else{
      digitalWrite(relayGPIOs[i-1], LOW);
    }
  }
  lcd.init();                   
  lcd.backlight();
  lcd.print("System Armed");
  lcd.clear();     
  Serial.print("Connecting to ");
  Serial.println(ssid);
  lcd.setCursor(0,0);  
  lcd.print("Connecting to ");
  lcd.setCursor(0,1);
  lcd.println(ssid);   
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
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?relay=<inputMessage>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    String inputMessage2;
    String inputParam2;
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) & request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
      if(RELAY_NO){
        Serial.print("NO ");
        digitalWrite(relayGPIOs[inputMessage.toInt()-1], !inputMessage2.toInt());
      }
      else{
        Serial.print("NC ");
        digitalWrite(relayGPIOs[inputMessage.toInt()-1], inputMessage2.toInt());
      }
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage + inputMessage2);
    request->send(200, "text/plain", "OK");
  });
  // Start server
  server.begin();
  timeClient.begin();
  lcd.clear();    
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
  lcd.setCursor(0,0); 
  lcd.print(ssid);    
  lcd.setCursor(0,1);   
  lcd.print(WiFi.localIP());   
  }
}
