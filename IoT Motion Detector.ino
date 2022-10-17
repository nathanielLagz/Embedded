#include <ESP8266WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <SinricProMotionsensor.h>

#define ssid "_Classified."
#define password "183768967"

#define appKey "5af02028-425a-430b-8700-5ac9c9701a8c"
#define appSecret "e7547b6c-b550-4899-85b8-fe9356433de0-8b8430fa-0cf0-417f-b2dc-eaeeda9aa628"
#define deviceID "633d51fa134b2df11cc36730"


bool powerState = true, lastMotionState = false, sent = false, alarm;
unsigned long lastChange0 = 0, lastChange1 = 0, duration;
int num = 0, distance;
int repeat = 9;

// Use D1 and D2 for relays
#define relay D1
#define buzzer D2
#define trig D5
#define echo D7
// #define sensor D8

bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Device %s turned %s (via SinricPro) \r\n", deviceId.c_str(), state?"on":"off");
  alarm = state;
  Serial.printf("Alarm is: %s\r\n", alarm?"on":"off");
  // digitalWrite()
  // digitalWrite(relay, state?LOW:HIGH);
  // powerState = state;
  return true;
}

void security() {
  unsigned long actualMillis = millis();
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  duration = pulseIn(echo, HIGH);
  distance = duration * 0.034 / 2;
  if (actualMillis - lastChange0 < 1250) return;
  
  SinricProSwitch& mySwitch = SinricPro[deviceID];
  if(distance <= 40) {
    if(repeat == 9 && !sent) {
      sent = true;
      mySwitch.sendPushNotification("Motion detected!");
      Serial.println("Sending notif");
    }
    Serial.println("Motion detected");
    digitalWrite(relay, HIGH);
  }
  else {
    digitalWrite(relay, LOW);
    Serial.println("Motion absent");    
  }
  if(sent){
    if(repeat == 0) {
      repeat = 9;
      sent = false;
    }
    else {
      repeat--;
    }      
  }
  lastChange0 = actualMillis; 
}

void setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(250);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  SinricProSwitch& myMotionsensor = SinricPro[deviceID];
  myMotionsensor.onPowerState(onPowerState);
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(appKey, appSecret);
}

void buzz() {
  unsigned long actualMillis1 = millis();
  if(!alarm) {
    noTone(buzzer);
    return;
  }
  if (actualMillis1 - lastChange1 < 1000) return;
  if(actualMillis1 / 1000 % 2 == 0) {
    Serial.println("Buzzing");  
    tone(buzzer, 1000);
  }
  else {
    Serial.println("Unbuzzing");      
    noTone(buzzer);
  }
  lastChange1 = actualMillis1;
}

void setup() {
  digitalWrite(D4, HIGH);
  Serial.begin(115200);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(D4, OUTPUT);
  setupWifi();
  digitalWrite(D4, LOW);
}

void loop() {
  SinricPro.handle();
  security(); 
  buzz();
}
