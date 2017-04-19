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
bool isActive = false;

/*******__MESSAGES__***********/
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
    Serial.println("local ip" + WiFi.localIP());
}

void setupMDNS() {
    // Add service to MDNS-SD to access the ESP with the URL http://<ssid>.local
    if (MDNS.begin(ssid)) {
        Serial.print("MDNS responder started as http://" + ssid + ".local");
    }
    MDNS.addService("http", "tcp", 8080);
}

void setup() {
    pinMode(debugLEDpin, OUTPUT);
    Serial.begin(115200);

    Serial.println("Starting WiFi.");
    setupWifi();
    setupServer();
    setupMDNS();
    Serial.println("Setup OK.");
    
}
bool runLED() {
  digitalWrite(debugLEDpin, isActive ? HIGH : LOW);  
}

void loop() {
    server.handleClient();
    runLED();
}

String getHTML(int page) {
    Serial.println("Call getHTML.");
    digitalWrite(debugLEDpin, LOW);  // (inverted logic)
    String html = "";
    if(page == PAGE_ROOT) {
      html = html + page_root;
    } else if(page == PAGE_TOGGLE) {
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
    Serial.println("Alam Form .");
    server.send(200, "text/html", getHTML(PAGE_TOGGLE));
}

/****Manage LEDs****/
void handleAlarmSetting() {
    if ( server.hasArg("toggle") ) {
        toggleAlarm();
        Serial.print("toggleAlarm ...");
    } else {
        Serial.println("Bad URL.");
        server.send(404, "text/plain", "Bad URL.");
        return;                                                     
    }
    Serial.println("success .");
    handleAlarm();
}

void setupServer() {
    server.on("/", handleRoot);
    server.on("/alarm", handleAlarm);
    server.on("/set", handleAlarmSetting);
    server.on("/log", handleRoot);
    server.begin();
    Serial.println("HTTP server started");
}


