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
#include <DHT.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

int STATE_BUTTON = 14;
int DHT_PIN = 12;
int BUZZER_PIN = 10;
//int BUTTON_ALARM = 12;

String curHour, curMinute, currentDay, curMonth, curYear;
String hourAlarmString, minuteAlarmString;

String weekDays[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
String months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

int hourAlarming;
int minuteAlarming;

// Delta Time
float current;
float oldTime;
float deltaTime;
float buttonTimer;

// Button
int state;
float timer;
bool isPressedButton;

// Alarm
int alarmState;
boolean isAlarming;
boolean isAlarmActivated;

#ifndef LED_BUILTIN
#define LED_BUILTIN 13 // ESP32 DOES NOT DEFINE LED_BUILTIN
#endif
int LED = LED_BUILTIN;

// DHT11 Sensor
DHT dht(DHT_PIN, DHT11);

// Icon
byte degreeIcon[] = {
  B11100,
  B10100,
  B11100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

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
  // Setup pin for required input and output
  dht.begin();
  pinMode(STATE_BUTTON, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  //pinMode(BUTTON_ALARM, OUTPUT);
  noTone(BUZZER_PIN);
  // put your setup code here, to run once:
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // put your setup code here, to run once:
  Serial.begin(115200);
  
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

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Initialize variables
  state = 1;
  buttonTimer = .5;
  isAlarming = false;
  isAlarmActivated = true;
  isPressedButton = false;
  alarmState = 1;
  
  timeClient.begin();
  timeClient.setTimeOffset(25200);

  // Setup for LCD_I2C
  Wire.begin(D2, D1);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, degreeIcon);
}

void loop() {
   oldTime = current;
   current = millis();
   deltaTime = (current - oldTime) / 1000;
   // put your main code here, to run repeatedly:
   timeClient.update();

   hourAlarming = 7;
   if (hourAlarming < 10) {
      hourAlarmString = "0" + String(hourAlarming);
   } else {
      hourAlarmString = String(hourAlarming);
   }
   
   minuteAlarming = 0;
   if (hourAlarming < 10) {
      minuteAlarmString = "0" + String(minuteAlarming);
   } else {
      minuteAlarmString = String(minuteAlarming);
   }
   
   int buttonState = digitalRead(STATE_BUTTON);
   Serial.println(buttonState);

   // Read Humidity and Temperature
   float humidity = dht.readHumidity();
   float temperature = dht.readTemperature();
   int int_humidity = (int) humidity;
   int int_temperature = (int) temperature;
   // Convert to String
   String h = String(int_humidity);
   String t = String(int_temperature);
   
   //int alarmButton = digitalRead(BUTTON_ALARM);
  
   unsigned long epochTime = timeClient.getEpochTime();
  
   struct tm *ptm = gmtime ((time_t *)&epochTime);

  // Get current time data from NTPClient
   int currentHour = timeClient.getHours();
   if (currentHour < 10) {
      curHour = "0" + String(currentHour);
   } else {
      curHour = String(currentHour);
   }
   
   int currentMinute = timeClient.getMinutes();
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

   //if (alarmButton == HIGH && isAlarming) {
   //   isAlarming = false;
   //}

   //if (hourAlarming == currentHour && minuteAlarming == currentMinute) {
   //   if (isAlarmActivated && isAlarming) {
   //       tone(BUZZER_PIN, 350, 5);
   //   } else if (!isAlarmActivated || !isAlarming ) {
   //       noTone(BUZZER_PIN);
   //   }
   //}

   String alarmHourDisplay = String(hourAlarming);
   String alarmMinuteDisplay = String(minuteAlarming);

   if (buttonState == HIGH && !isPressedButton) {
      isPressedButton = true;
      lcd.clear();
      Serial.println("Switch state");
      switch (state) {
         case 1:
           state = 2;
           break;
         case 2:
           state = 1;
           break;
      }
      
   } else if (buttonState == LOW) {
      switch (state) {
        case 1:
          // Set Date (First Line)
          lcd.setCursor(1, 0);
          lcd.print(weekDay);
          lcd.setCursor(5, 0);
          lcd.print(currentDay);
          lcd.setCursor(7, 0);
          lcd.print("/");
          lcd.setCursor(8, 0);
          lcd.print(curMonth);
          lcd.setCursor(10, 0);
          lcd.print("/");
          lcd.setCursor(11, 0);
          lcd.print(curYear);

          // Set Time (Second Line)
          lcd.setCursor(1, 1);
          lcd.print(h);
          lcd.setCursor(3, 1);
          lcd.print("%");
          lcd.setCursor(5, 1);
          lcd.print(t);
          lcd.setCursor(7, 1);
          lcd.write(byte(0));
          lcd.setCursor(8, 1);
          lcd.print("C");
          lcd.setCursor(10, 1);
          lcd.print(curHour);
          lcd.setCursor(12, 1);
          lcd.print(":");
          lcd.setCursor(13, 1);
          lcd.print(curMinute);
          break;
          
        case 2:
          // Show Alarm Time
          lcd.setCursor(3, 0);
          lcd.print("Alarm Time");
          lcd.setCursor(6, 1);
          lcd.print(hourAlarmString);
          lcd.print(":");
          lcd.print(minuteAlarmString);
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}
