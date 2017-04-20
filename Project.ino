/****Includes****/
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the &onfiguration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266mDNS.h>          //Allow custom URL

/*****Initialization*****/
ESP8266WebServer server(80);
const char *ssid = "Riot_intro_ini";
const int debugLEDpin = D4;
const int processLEDpin = D3;
const int alertLEDpin = D1;
const int buzzer = D6;
String password;
bool isActive = false;
bool inProcess = false;
bool inAlarm = false;
int sensor;           //Variable to store analog value (0-1023)

/*******__PAGES__**************/
int PAGE_ROOT = 0;
int PAGE_TOGGLE = 1;
/*******__WEB_PAGE__***********/
String page_struct = "<!doctype html> <html> <head> <title> IoT into </title> </head> <body>\
{%CONTENT%}\
</body> </html>\
";
String page_root = "\
<p>{%CURRENT_STATE%}</p>\
<p><a href='/alarm'>Settings</a></p>\
<p><a href='/log'>Logs</a></p>\
\
";
String page_toggleAlarme  = "\
<p>{%CURRENT_STATE%}</p>\
<form method='get' action='/set'>\
<button type='submit' name='toggle' value='alarm'>{%STATUS%}</button>\
</form>\
";

/*******__MESSAGES__***********/
String active = "Active";
String desactive = "Desactive";
String toggle_activation = "Activation";
String toggle_desactivation = "Desactivation";

bool toggleAlarm() {
  isActive = !isActive;
  inAlarm = false;
}

String currentState() {
  return isActive ? active : desactive ;
}

String currentButtonToggle() {
  return isActive ? toggle_desactivation : toggle_activation ;
}

//gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager) {
  // LEDfeedback(RED); // waiting for connection
}

void setupWifi() {
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect(ssid);
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}

void setupMDNS() {
  // Add service to MDNS-SD to access the ESP with the URL http://<ssid>.local
  if (MDNS.begin(ssid)) {
    Serial.print("MDNS responder started as http://");
    Serial.print(ssid);
    Serial.println(".local");
  }
  MDNS.addService("http", "tcp", 8080);
}

void setup() {
  pinMode(debugLEDpin, OUTPUT);
  pinMode(alertLEDpin, OUTPUT);
  pinMode(processLEDpin, OUTPUT);
  Serial.begin(115200);

  Serial.println("Starting WiFi.");
  setupWifi();
  setupServer();
  setupMDNS();
  Serial.println("Setup OK.");

}
bool runLED() {
  digitalWrite(processLEDpin, inProcess ? HIGH : LOW);
  digitalWrite(debugLEDpin, isActive ? HIGH : LOW);
  digitalWrite(alertLEDpin, isActive && inAlarm ? HIGH : LOW);
}

bool runAlarm() {
  runLED();
  if (isActive && inAlarm) {
    tone(buzzer, 500);
  } else if (isActive) {
    sensor = analogRead(A0);
    //While sensor is not moving, analog pin receive 1023~1024 value
    if (sensor < 1022) {
      inAlarm = true;
    }
  } else {
    noTone(buzzer);
  }
}

void loop() {
  server.handleClient();
  runAlarm();
}

String getHTML(int page) {
  Serial.println("Call getHTML.");
  digitalWrite(debugLEDpin, LOW);  // (inverted logic)
  String html = "";
  if (page == PAGE_ROOT) {
    html = html + page_root;
  } else if (page == PAGE_TOGGLE) {
    html = html + page_toggleAlarme;
    html.replace("{%STATUS%}", currentButtonToggle());
  }
  html.replace("{%CURRENT_STATE%}", currentState());

  String html_struct = "" + page_struct;
  html_struct.replace("{%CONTENT%}", html);

  return html_struct;
}

void handleRoot() {
  Serial.println("Call handleRoot.");
  server.send(200, "text/html", getHTML(PAGE_ROOT));
}

void handleAlarm() {
  Serial.print("Call handleAlarm ...");
  Serial.println("Alam Form .");
  server.send(200, "text/html", getHTML(PAGE_TOGGLE));
}

/****Manage LEDs****/
void handleAlarmSetting() {

  Serial.print("Call handleAlarmSetting ...");
  if ( server.hasArg("toggle") ) {
    Serial.print("toggleAlarm ...");
    toggleAlarm();
  } else {
    Serial.println("Bad URL.");
    server.send(404, "text/plain", "Bad URL.");
    return;
  }
  Serial.println("success .");
  handleAlarm();
}

void handleError() {
  server.send(404, "text/plain", "Bad URL.");
}

void setupServer() {
  server.on("/", handleRoot);
  server.on("/alarm", handleAlarm);
  server.on("/set", handleAlarmSetting);
  server.on("/log", handleRoot);
  server.onNotFound(handleError);
  server.begin();
  Serial.println("HTTP server started");
}


