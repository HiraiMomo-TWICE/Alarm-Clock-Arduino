//for LED status
#include <Ticker.h>
Ticker ticker;

// Other libraries
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Read PIN of sensor, buttons
LiquidCrystal_I2C lcd(0x27, 16, 2);

#ifndef LED_BUILTIN
#define LED_BUILTIN 13 // ESP32 DOES NOT DEFINE LED_BUILTIN
#endif

int LED = LED_BUILTIN;
String weekDays[7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
String months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

// Delta Time
float current;
float oldTime;
float deltaTime;
float buttonTimer;

// Button
int state;
float timer;
bool isPressedButton;
String curHour, curMinute, currentDay, curMonth, curYear;

void tick()
{
  //toggle state
  digitalWrite(LED, !digitalRead(LED));     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void setup() {
  WiFi.disconnect(true);
  // put your setup code here, to run once:
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  //set led pin as output
  pinMode(LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  //reset settings - for testing
  // wm.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wm.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wm.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  ticker.detach();
  //keep LED on
  digitalWrite(LED, LOW);

  Wire.begin(D2, D1);

  state = 1;
  buttonTimer = .5;
  isPressedButton = false;
  
  timeClient.begin();
  timeClient.setTimeOffset(25200);
  
  lcd.init();
  lcd.backlight();
}

void loop() {
  // put your main code here, to run repeatedly:
  // Update deltatime
  oldTime = current;
  current = millis();
  deltaTime = (current - oldTime) / 1000;
  
  timeClient.update();

  // Get current time data from NTPClient
   int currentHour = timeClient.getHours();
   if (currentHour < 10) {
    curHour = "0" + String(currentHour);
   } else {
    curHour = String(currentHour);
   }
   
   int currentMinute = timeClient.getMinute();
   if (currentMinute < 10) {
    curMinute = "0" + String(currentMinute);
   } else {
    curMinute = String(currentMinute);
   }
   
   String weekDay = weekDays[timeClient.getDay()];
   
   int monthDay = ptm->tm_mday;
   if (monthDay < 10) {
    currentDay = "0" + String(monthDay);
   } else {
    currentDay = String(monthDay);
   }
   
   int currentMonth = ptm->tm_mon+1;
   if (currentMonth < 10) {
    curMonth = "0" + String(currentMonth);
   } else {
    curMonth = String(currentMonth);
   } 
   
   int currentYear = ptm->tm_year+1900;
   curYear = String(currentYear);
   
   if (buttonState == HIGH && !isPressedButton) {
    isPressedButton = true;
    lcd.clear();
    
    switch (state) {
      case 1:
        state = 2;
        break;
      case 2:
        state = 1;
        break;
   } else if (buttonState == LOW) {
    switch (state) {
      case 1:
        // Set Date
        lcd.setCursor(1, 0);
        lcd.print("%s %s/%s/%s", weekDay, currentDay, curMonth, curYear);
         
        // Set humidity, set temperature, set Time
        lcd.setCursor(1, 1);
        lcd.print("");
        break;
      case 2:
        // Show Alarm Time
        lcd.setCursor(3, 0);
        lcd.print("Alarm Time");
        lcd.setCursor(
        break;
    }
   }  

  if (isPressedButton) {
     timer += deltaTime;
     
     if (timer >= buttonTimer) {
        timer = 0;
        isPressedButton = false;
     }
  }
}
