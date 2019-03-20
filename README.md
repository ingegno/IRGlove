# IRGlove
Adaptatie van de GloveIR arduino code: a handschoen om IR toestellen te controleren.

## Base project
* [Phablabs.eu ir-glove project](http://phablabs.eu/workshop/ir-glove)
* [PDF Constructie handleiding](http://phablabs.eu/sites/default/files/Photonics%20IRglove_online_0.pdf)

Code is gebaseerd op record.ino van [IRLib2](https://github.com/cyborg5/IRLib2) welke GPL v3 is, dus deze code is ook GPL v3

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

### Benodigdheden
Installeer op je computer de Arduino IDE 1.8.8. 

Installeer dan via de library manager (`CTRL+SHIFT+I`) volgende bibliotheek:
* EEPROMEx versie 1.0.0 van Thijs Elenbaas

![Install EEPROMex](doc/fig/EEPROMex_lib.png)

Vervolgens download via deze link [IRLib2](https://github.com/cyborg5/IRLib2) via de **Download Zip** knop (Wij gebruiken versie 3 Jun 2018). VOEG HIER SCREENSHOT TOE
Unzip de download, en kopieer **een deel** van de Download naar de Arduino library folder zodat deze volgende structuur heeft: 

    Arduino/libraries/IRLib2
    Arduino/libraries/IRLibFreq
    Arduino/libraries/IRLibProtocols
    Arduino/libraries/IRLibRecv
    Arduino/libraries/IRLibRecvPCI

Op Windows is dit normaal in locatie `C:\Users\USERNAME\Documents\Arduino`, op linux in `\home\USERNAME\Arduino`.

### IRGlove code
Download de code van deze git via de **Download zip** knop en unzip de download. 
SCREENSHOT
Dubbel klik op de `IRGlove-master/IRGlove/IRGlove.ino` file in de unzipped code folder. Dit zal Arduino 1.8.8 openen op onze code. 

Selecteer als bord Arduino Nano. Druk op compileer om te code te verifieren. 

Krijg je de fout:

    lto1: internal compiler error: in lto_output_varpool_node, at lto-cgraph.c:624
    Please submit a full bug report,
    with preprocessed source if appropriate.

dan dien je oplossing uit [deze thread](https://forum.arduino.cc/index.php?topic=574020.0) uit te voeren als gevolg van bug [39](https://github.com/arduino/ArduinoCore-avr/issues/39):

    Tools > Board > Boards Manager
    Wait for downloads to finish. WAT IS DIT WACHTEN????
    When you move the mouse pointer over "Arduino AVR Boards", you will see a "Select version" dropdown menu appear. Select "1.6.21". SCREENSHOT
    Click "Install".
    Wait for installation to finish.
    Click "Close".

Na dit project kun je opnieuw de laatste versie van de *Arduino AVR Boards* installeren.
