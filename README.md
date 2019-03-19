# IRGlove
Adaptatie van de GloveIR arduino code: a handschoen om IR toestellen te controleren.

## Base project
* [Phablabs.eu ir-glove project](http://phablabs.eu/workshop/ir-glove)
* [PDF Constructie handleiding](http://phablabs.eu/sites/default/files/Photonics%20IRglove_online_0.pdf)

Code is gebaseerd of record.ino van [IRLib2](https://github.com/cyborg5/IRLib2) welke GPL v3 is, dus deze code is ook GPL v3

## Constructie
We doen volgende wijzigingen tov het Phalabs project
* gebruik Arduino Nano
* voeg een OLED scherm toe
* voeg een drukknop toe

# Handleiding

## Constructie op Breadboad
Maak eerst de constructie op een breadboard. Daarna laden we de code op naar de Arduino, en testen.

*Hier komt foto schema en breadboard constructie*

## Code Installeren
Installeer op je computer de Arduino IDE 1.8.8. 

Installeer dan via de library manager volgende bibliotheek:
* EEPROMEx versie 1.0.0 van Thijs Elenbaas

Download vervolgens [IRLib2](https://github.com/cyborg5/IRLib2) via de **Download Zip** knop (Wij gebruiken versie 3 Jun 2018). Unzip de download, en kopieer **een deel** van de Download naar de Arduino library folder zodat deze volgende structuur heeft: 

    Arduino/libraries/IRLib2
    Arduino/libraries/IRLibFreq
    Arduino/libraries/IRLibProtocols
    Arduino/libraries/IRLibRecv
    Arduino/libraries/IRLibRecvPCI



