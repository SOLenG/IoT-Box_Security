#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the &onfiguration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266mDNS.h>          //Allow custom URL

typedef enum color_e {RED, GREEN, BLUE, YELLOW, OFF} color_t;
const int RGBpins[] = {D8, D6, D7};
const int debugLEDpin = D4;
const int buzzer = D3;

/*****Initialization*****/
ESP8266WebServer server(80);
const char *ssid = "Box_Security";

/*****WebPage*****/
// Warning: only use simple quotes in the html (no double)
String rootHTML = "\
<!doctype html> <html> <head> <title> Box Security </title> </head> <body>\
<br> <br> Analog input: xxx V (<a href='/'>refresh<a>)\
<form method='get' action='/buzz'>\
  <br><br> <button type='submit' name='etatBuzz' value='on'>  Activation Alarme  </button>\
</form>\
<form method='get' action='/debuzz'>\
  <br><br> <button type='submit' name='etatBuzz' value='off'>  Desactivation Alarme  </button>\
</form>\
<form method='get' action='/journal'>\
  <br><br> <button type='submit'>  Journal evenements  </button>\
</form>\
<br> No .\
</body> </html>\
";

String getHTML() {
    digitalWrite(debugLEDpin, LOW);  // (inverted logic)

    String updatedRootHTML = rootHTML;
    String voltage = String(analogRead(A0) * 3. / 1024.);
    updatedRootHTML.replace("xxx", voltage);

    digitalWrite(debugLEDpin, HIGH); // (inverted logic)
    return updatedRootHTML;
}

void handleRoot() {
    server.send(200, "text/html", getHTML());
}

/****Manage LEDs****/
void handleLEDs() {
    String color_str;
    if ( server.hasArg("toggle") ) {
        color_str = server.arg(0);
        LEDtoggle(color_str[0]);
    } else {
        Serial.println("Bad URL.");
        server.send(404, "text/plain", "Bad URL.");
        return;
    }
    String answer = getHTML();
    answer.replace("No", color_str);
    server.send(200, "text/html", answer);
}

/****Manage Buzzer ***/

void activateBuzzer() {
 if ( server.hasArg("etatBuzz") ) { 
  tone(buzzer, 1000); 
  Serial.println("Buzzer activé");
 }
 else {
  Serial.println("Bad URL.");
        server.send(404, "text/plain", "Bad URL.");
        return;
 }
 String answer = getHTML();
 answer.replace("No", "Buzzer ON");
 server.send(200, "text/html", answer);
}

void desactivateBuzzer() {
 if ( server.hasArg("etatBuzz") ) { 
  noTone(buzzer); 
 }
 else {
  Serial.println("Bad URL.");
        server.send(404, "text/plain", "Bad URL.");
        return;
 }
 String answer = getHTML();
 answer.replace("ON", "OFF");
 server.send(200, "text/html", answer);
}

/*** Manage Journal ****/

void lectureJournal() {
  Serial.println("Vous êtes dans le journal d'évenements");
}

/*** Manage Capteurs ***/
/****Setups****/

//gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager) {
    Serial.println("attente de connexion");
}

void setupWifi() {
    //WiFiManager
    WiFiManager wifiManager;

    //reset saved settings -- Flush flash
    //wifiManager.resetSettings();

    //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wifiManager.setAPCallback(configModeCallback);

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect(ssid);

    // might seem redundant but it's not printed the 1st time:
    Serial.println("local ip");
    Serial.println(WiFi.localIP());
}

void setupServer() {
    server.on("/", handleRoot);
    server.on("/set", handleLEDs);
    server.on("/buzz", activateBuzzer);
    server.on("/debuzz", desactivateBuzzer);
    server.on("/journal", lectureJournal);
    server.begin();
    Serial.println("HTTP server started");
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
    digitalWrite(debugLEDpin, HIGH);  // (inverted logic)
    pinMode(buzzer, OUTPUT); // Sortie du Buzzer
    
    for (int i=0; i<3; i++)
        pinMode(RGBpins[i], OUTPUT);

    Serial.begin(115200);

    Serial.println("Starting LEDs.");
    LEDfeedback(YELLOW); // state feedback

    Serial.println("Starting WiFi.");
    setupWifi();
    setupServer();
    setupMDNS();

    Serial.println("Setup OK.");
    LEDfeedback(OFF); // ready
    digitalWrite(debugLEDpin, LOW);  // (inverted logic)
}

/****Loop****/
void loop() {
    server.handleClient();
}

/****LEDs****/
bool RGBstates[3];
const float RGBintensities[] = {0xFF, 0xFF*0.3, 0xFF*0.6};

void LEDtoggle(char color) {
    int i = 0;
    switch (color) {
        case 'R' : i = 0; break;
        case 'G' : i = 1; break;
        case 'B' : i = 2; break;
        default:
            Serial.print("LEDtoggle() switch failed!");
            return;
    }
    RGBstates[i] ^= 1; // toggle
    analogWrite(RGBpins[i], RGBstates[i]*RGBintensities[i]);
}

void LEDfeedback(color_t color) {
    switch (color) {
        case RED :    RGBstates[0]=1; RGBstates[1]=0; RGBstates[2]=0; break;
        case GREEN :  RGBstates[0]=0; RGBstates[1]=1; RGBstates[2]=0; break;
        case BLUE :   RGBstates[0]=0; RGBstates[1]=0; RGBstates[2]=1; break;
        case YELLOW : RGBstates[0]=1; RGBstates[1]=1; RGBstates[2]=0; break;
        case OFF :    RGBstates[0]=0; RGBstates[1]=0; RGBstates[2]=0; break;
        default:
            Serial.print("LEDfeedback() switch failed!");
            return;
    }
    for (int i=0; i<3; i++)
        analogWrite(RGBpins[i], RGBstates[i]*RGBintensities[i]);
}
