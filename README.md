# IoT - Box Security

 - ESGI Projet
 
Goal
--------

 Create an alarm security system for a small container controlled by a web interface to dis/enable, send a issue notification and view the latest event logs. 
 
Features
---------------

- Mailing
- Interface web
  - Dis/enable alarm by user authenticated
  - Logs
- Alarm :
  - Sound Alert
  - States vizualisation

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

