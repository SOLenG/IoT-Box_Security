#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the &onfiguration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266mDNS.h>          //Allow custom URL
#include "Gsender.h"             //Bibliothèque pour envoi de mail (Gmail)

const int buzzer = D3;     //Sortie Buzzer
int photocellPin = 0;     // the cell and 10K pulldown are connected to a0
int photocellReading;     // the analog reading from the analog resistor divider
bool isMailLight = false;

/*****Initialization*****/
ESP8266WebServer server(80);
const char *ssid = "Box_Security";

/*****WebPage*****/
// Warning: only use simple quotes in the html (no double)
String rootHTML = "\
<!doctype html> <html><center> <head> <title> Box Security </title> </head></center><center> <body>\
<table>\
<tr><form method='get' action='/journal'>\
  <br><br> <button type='submit'>  Journal evenements  </button>\
  <br><br> <input type='text' name='event'> \
</form></tr> \
<tr><br> Etat du coffre : Securise .\ 
</tr> </table>\
</body></center> </html>\
";

String getHTML() {
    String updatedRootHTML = rootHTML;
    String voltage = String(analogRead(A0) * 3. / 1024.);
    updatedRootHTML.replace("xxx", voltage);
    return updatedRootHTML;
}

void handleRoot() {
    server.send(200, "text/html", getHTML());
}

/****Manage Buzzer ***/

/*void activateBuzzer() {
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
 answer.replace("No", "Buzzer OFF");
 server.send(200, "text/html", answer);
}*/

void activateTest()
{
  tone(buzzer, 1000); 
  Serial.println("Buzzer activé");
  String answer = getHTML();
  answer.replace("Securise", "Non securise");
  server.send(200, "text/html", answer);
}

void desactivateTest()
{
  noTone(buzzer); 
  String answer = getHTML();
  answer.replace("Securise", "Securise");
  server.send(200, "text/html", answer);
  isMailLight = true;
}
/*** Manage Journal ****/

void lectureJournal() {
  Serial.println("Vous êtes dans le journal d'évenements");
}

/*** Manage Capteurs ***/

//Capteur de luminosité 

int lightSensorRead()
{
   photocellReading = analogRead(photocellPin);  
 
//  Serial.print("Analog reading = ");
//  Serial.print(photocellReading);     

  if (photocellReading < 10) {
 //   Serial.println(" - Dark");
  } else if (photocellReading < 200) {
//    Serial.println(" - Dim");
  } else if (photocellReading < 500) {
  //  Serial.println(" - Light");
    //journal évenements : ouvert
    
  } else if (photocellReading < 800) {
  //Journal des évenements :
    
  //Mail :
    if(isMailLight == false)
    {
     Serial.println(" mail light ok");
    }
    else
    {
     sendMailLumiere();
     isMailLight = false;
     Serial.println(" mail light KO"); 
    }
  } else {
   //Serial.println(" - Very bright");
    //journal évenements : ouvert
  }
  return photocellReading;
}
/*** Tools ***/

void sendMailLumiere()
{
    Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    String subject = "ALERTE : BOITE OUVERTE";
    if(gsender->Subject(subject)->Send("gamelinfabien@gmail.com", "ALERTE : Votre boîte a été ouverte !")) {
        Serial.println("Message send.");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
    }
}
/****Setups****/

//gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager) {
    Serial.println("attente de connexion");
}

void setupWifi() {
    WiFiManager wifiManager;
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.autoConnect(ssid);
    Serial.println("local ip");
    Serial.println(WiFi.localIP());
}

void setupServer() {
    server.on("/", handleRoot);
  //  server.on("/buzz", activateBuzzer);
  //  server.on("/debuzz", desactivateBuzzer);
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

    pinMode(buzzer, OUTPUT); // Sortie du Buzzer
    Serial.begin(115200);
    Serial.println("Starting WiFi.");
    setupWifi();
    setupServer();
    setupMDNS();

 /*   Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    String subject = "Subject is optional!";
    if(gsender->Subject(subject)->Send("gamelinfabien@gmail.com", "COUCOU")) {
        Serial.println("Message send.");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
    }*/

    Serial.println("Setup OK.");
}

/****Loop****/
void loop() {
    server.handleClient();
    //gérer capteur de lumière
    if(lightSensorRead()>= 500)
    {
      activateTest();
    }
    desactivateTest();
    //gérer capteur déplacement
}

