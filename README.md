# IoT-intro

 - ESGI Projet
 
Objectif
--------

 Créer un système de sécurité par alarme pour un conteneur de petite taille, contrôlé via une interface WEB permettant de l’activer, désactiver, émettre des notification sur des événements et la consultation des derniers événement au travers d’un journal.
 
Fonctionnalités
---------------

List features :
- Mailing
- Interface web
  - Dés/Activation de l’alarme par saisie d’un code
  - Consultation du journal des événement
- Alarme :
  - Emission sonore
  - Visualisation de l’état d’activation
  - Temporisation avant déclenchement de l’alarme (emission sonore) pour la saisie du code

Composants nécessaires 
----------------------

- 1 ESP8266
- 1 Light sensor (open box)
- 1 Tilts sensor (move box)
- 1 Buzzer (Sound alert)
- 3 LED (States vizualisation)
- 1 Resistance of 10k ohms

![SCHEMA](https://image.noelshack.com/fichiers/2017/11/1489757082-schema.png)

# Change Logs :

* Add WifiManager
* Add Web interface 
  * enable/disable alarm
  * LED state alarm
* Add Buzzer
* Add tilts sensor
* Add LEDs
  * Warning alarm
  * Web server on/off
* Add Authentification for web interface

# Todo :
* Design of web interface
* Logs access
* reset wifi config by web interface
* Config mail target by web interface

