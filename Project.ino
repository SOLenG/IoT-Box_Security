/****Includes****/
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the &onfiguration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266mDNS.h>          //Allow custom URL
#include "Gsender.h"             //Bibliothèque pour envoi de mail (Gmail)

/*****Initialization*****/
ESP8266WebServer server(80);
const char *ssid = "Riot_intro_ini";
const int alertLEDpin = D1;      // This LED return the state of alarm (active/inactive)
const int processLEDpin = D3;    // This LED return the state of server (active/inactive)
const int debugLEDpin = D4;      // This LED return the state of alarm (armed/disable)
const int buzzer = D6;           // Sortie Buzzer
const int sensorPin = D7;        // Movement sensor
const int photocellPin = A0;     // the cell and 10K pulldown are connected to a0
bool isActive = false;           // the state of alarm (active/inactive)
bool inProcess = false;          // the state of server (active/inactive)
bool inAlarm = false;            // the state of server (active/inactive)
int sensor;                      // Variable to store analog value (0-1023)
int photocellReading;            // the analog reading from the analog resistor divider
bool isMailLight = false;        // limit the number send mail

/******__AUTHENTICATE__********/
String www_username = "admin";   // username for authenticate
String www_password = "admin";   // password for authenticate

/*******__PAGES__CODE__********/
const int PAGE_ROOT = 0;         // Main page
const int PAGE_TOGGLE = 1;       // Alarm setting page

/*******__HTML_PAGE__**********/
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
String page_toggleAlarm = "\
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

/**
   ARDUINO SYSTEM
*/
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

void loop() {
  server.handleClient();
  runAlarm();
  delay(1);
}

/***************______SETUP_____**********************/
void setupWifi() {
  WiFiManager wifiManager;
  //  wifiManager.resetSettings();
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

void setupServer() {
  server.on("/", handleRoot);
  server.on("/alarm", handleAlarm);
  server.on("/login", handleLogin);
  server.on("/logout", handleLogout);
  server.on("/set", handleAlarmSetting);
  server.on("/log", handleRoot);
  server.onNotFound(handleError);

  const char *headerkeys[] = {"User-Agent", "Cookie"};
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char *);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize);

  server.begin();
  inProcess = true; // waiting for connection
  runLED();
  Serial.println("HTTP server started");
}

/**
   RUN System (in loop)
*/
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

    Serial.print("sensor      :");
    Serial.println(sensor);
    if (sensor < 1022 || lightSensorRead() >= 500) {
      inAlarm = true;
    }
  } else {
    noTone(buzzer);
  }
}

/**
   Template Engine
*/
String getHTML(int page) {
  Serial.println("Call getHTML.");
  digitalWrite(debugLEDpin, LOW);  // (inverted logic)

  String html = "";
  if (page == PAGE_ROOT) {
    html = html + page_root;
  } else if (page == PAGE_TOGGLE) {
    html = html + page_toggleAlarm;
    html.replace("{%STATUS%}", currentButtonToggle());
  }
  html.replace("{%CURRENT_STATE%}", currentState());

  String html_struct = "" + page_struct;
  html_struct.replace("{%CONTENT%}", html);

  return html_struct;
}

/**
   Security
*/
void _authenticate() {
  server.sendHeader("Location", "/login");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(301);
  return;
}

/**
   CONTROLLER (Handle_)
*/
void handleRoot() {
  if (!is_authentified()) {
    _authenticate();
    return;
  }

  Serial.println("Call handleRoot.");
  server.send(200, "text/html", getHTML(PAGE_ROOT));
}


/** Alarm form Settings Controller **/
void handleAlarm() {
  if (!is_authentified()) {
    _authenticate();
    return;
  }
  Serial.print("Call handleAlarm ...");
  Serial.println("Alam Form .");
  server.send(200, "text/html", getHTML(PAGE_TOGGLE));
}

/** Alarm Alter Settings Controller **/
void handleAlarmSetting() {

  if (!is_authentified()) {
    _authenticate();
    return;
  }
  Serial.print("Call handleAlarmSetting ...");
  if (server.hasArg("toggle")) {
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

/** Error Page Controller**/
void handleError() {
  server.send(404, "text/plain", "Bad URL.");
}


/** Login Controller **/
void handleLogin() {
  String msg;
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")) {
    if (server.arg("USERNAME") == www_username && server.arg("PASSWORD") == www_password) {
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
      server.send(301);
      Serial.println("Log in Successful");
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
  }
  String content = "<html><body><form action='/login' method='POST'>To log in, please use : admin/admin<br>";
  content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  content += "</body></html>";
  server.send(200, "text/html", content);
}

/** LogOut Controller **/
void handleLogout() {
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
    server.send(301);
    return;
  }
}

/**
   Mailings system
*/
void sendMailLumiere() {
  Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
  String subject = "ALERTE : BOITE OUVERTE";
  if (gsender->Subject(subject)->Send("gamelinfabien@gmail.com", "ALERTE : Votre boîte a été ouverte !")) {
    Serial.println("Message send.");
  } else {
    Serial.print("Error sending message: ");
    Serial.println(gsender->getError());
  }
}

void sendMailDeplacement() {
  Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
  String subject = "ALERTE : BOITE DEPLACEE";
  if (gsender->Subject(subject)->Send("gamelinfabien@gmail.com", "ALERTE : Votre boîte a été déplacée !")) {
    Serial.println("Message send.");
  } else {
    Serial.print("Error sending message: ");
    Serial.println(gsender->getError());
  }
}

void sendMailPresence() {
  Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
  String subject = "ALERTE : PRESENCE AUTOUR DE LA BOITE";
  if (gsender->Subject(subject)->Send("gamelinfabien@gmail.com",
                                      "ALERTE : Une présence a été detectée autour de votre boîte !")) {
    Serial.println("Message send.");
  } else {
    Serial.print("Error sending message: ");
    Serial.println(gsender->getError());
  }
}

/**
   Sensor Reader
*/
int lightSensorRead() {
  /*photocellReading = analogRead(photocellPin);
          Serial.print("photocellReading : ");
          Serial.println(photocellReading);
    if (photocellReading < 800) {
      //Mail :
      if (!isMailLight) {
          Serial.println(" fin d'envoi mail");
      } else {
          sendMailLumiere();
          isMailLight = false;
          Serial.println("Mail envoyé ! ");
      }
    }*/

  return 0;//photocellReading;
}

/**
   Tools function
*/
bool is_authentified() {
  Serial.println("Enter is_authentified");
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentification Successful");
      return true;
    }
  }
  Serial.println("Authentification Failed");
  return false;
}
bool toggleAlarm() {
  isActive = !isActive;
  inAlarm = false;
}

String currentState() {
  return isActive ? active : desactive;
}

String currentButtonToggle() {
  return isActive ? toggle_desactivation : toggle_activation;
}

