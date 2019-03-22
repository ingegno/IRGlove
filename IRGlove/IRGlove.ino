/* Based on record.ino Example sketch from IRLib2 - GPL v3
 *  Adapted by VUB http://phablabs.eu/workshop/ir-glove 
 *  Adapted by Ingegno Maker Space:
 *      - fix bugs
 *      - correct store of signals
 *      
*/

/*  USER SETTINGS 
 *   
*/
// board selection
#define B_UNO 0
#define B_NANO 1
// do you use an UNO or a NANO for the glove ? 
#define IRGLOVE_BOARD B_NANO

// select language
#define LANG_EN 0
#define LANG_NL 1
// select language of the output in serial monitor
#define LANG_OUTPUT LANG_NL

// set the fingers you will use
const int COUNT = 3;
const String vinger_EN[COUNT] = {"forefinger", "middle finger", "ring finger"};
const String vinger[COUNT] = {"wijsvinger", "middelvinger", "ringvinger"};
// pins on the Arduino for the fingers:
const int INPUTS[COUNT] = {8, 9, 10};

// optional pin to program
const int PROGRAM_PIN = 7;

// pin used by the IR receiver
const int IR_RECEIVE_PIN = 12;

// pin used by the IR sender is based on your hardware. UNO and NANO are pin 3
// for other boards look up the pin in IRLibProtocols/IRLibHardware.h of the library!

// pin with LED output to indicate programming state or not. Default is builtinLED pin 13
const int builtInLed = 13;

// set serial output on or off
#define TEST_WITH_SERIAL false

/*  CODE 
 *   
*/
int program_state;
const int programbtn = PROGRAM_PIN;
boolean builtInLed_ON = HIGH;
// there must be at least one finger. In setup we correct this if more!
int finger1 = INPUTS[0];
int finger2 = INPUTS[0];
int finger3 = INPUTS[0];

boolean programbtn_PRESSED = LOW;

boolean finger1_PRESSED = LOW;

boolean finger2_PRESSED = LOW;

boolean finger3_PRESSED = LOW;

const int maxAllowedWrites = 80;
// start reading from the first byte (address 120) of the EEPROM
const int memBase          = 120;

#include <EEPROMex.h>
#include <EEPROMVar.h>

#include <IRLibDecodeBase.h>  //We need both the coding and
#include <IRLibSendBase.h>    // sending base classes
#include <IRLib_P01_NEC.h>    //Lowest numbered protocol 1st
#include <IRLib_P02_Sony.h>   // Include only protocols you want
#include <IRLib_P03_RC5.h>
#include <IRLib_P04_RC6.h>
#include <IRLib_P05_Panasonic_Old.h>
#include <IRLib_P07_NECx.h>
#include <IRLib_HashRaw.h>    //We need this for IRsendRaw
#include <IRLibCombo.h>       // After all protocols, include this
// All of the above automatically creates a universal decoder
// class called "IRdecode" and a universal sender class "IRsend"
// containing only the protocols you want.
// Now declare instances of the decoder and the sender.

IRdecode myDecoder;
IRsend mySender;

// Include a receiver either this or IRLibRecvPCI or IRLibRecvLoop
#include <IRLibRecv.h>
IRrecv myReceiver(IR_RECEIVE_PIN); //pin number for the receiver

int programCode = 0;

// Storage for the recorded code
uint8_t codeProtocol[COUNT] = {0};  // The type of code
uint32_t codeValue[COUNT] = {0};    // The data bits if type is not raw
uint8_t codeBits[COUNT] = {0};      // The length of the code in bits

long programbtnbuttonTimer = 0;
#define programbtnminShortPressTime 80
#define programbtnlongPressTime 750
boolean programbtnbuttonActive = false;
boolean programbtnlongPressActive = false;
#define programbtnNOPRESS    0
#define programbtnSHORTPRESS 1
#define programbtnLONGPRESS  2
int programbtnPressType = programbtnNOPRESS;
long finger1buttonTimer = 0;
#define finger1minShortPressTime 80
#define finger1longPressTime 750
boolean finger1buttonActive = false;
boolean finger1longPressActive = false;
#define finger1NOPRESS    0
#define finger1SHORTPRESS 1
#define finger1LONGPRESS  2
int finger1PressType = finger1NOPRESS;
long finger2buttonTimer = 0;
#define finger2minShortPressTime 80
#define finger2longPressTime 750
boolean finger2buttonActive = false;
boolean finger2longPressActive = false;
#define finger2NOPRESS    0
#define finger2SHORTPRESS 1
#define finger2LONGPRESS  2
int finger2PressType = finger2NOPRESS;
long finger3buttonTimer = 0;
#define finger3minShortPressTime 80
#define finger3longPressTime 750
boolean finger3buttonActive = false;
boolean finger3longPressActive = false;
#define finger3NOPRESS    0
#define finger3SHORTPRESS 1
#define finger3LONGPRESS  2
int finger3PressType = finger3NOPRESS;

int ard_effect0_status = -1;
unsigned long ard_effect0_start, ard_effect0_time;
#define EFFECT0_PERIOD 500
#define EFFECT0_1_DURATION 250

void setup() {
  // start reading from position memBase (address 0) of the EEPROM. Set maximumSize to EEPROMSizeUno 
  // Writes before membase or beyond EEPROMSize will only give errors when _EEPROMEX_DEBUG is set
  if (IRGLOVE_BOARD == B_NANO) {
    EEPROM.setMemPool(memBase, EEPROMSizeNano);
  } else {
    EEPROM.setMemPool(memBase, EEPROMSizeUno);
  }
  // Set maximum allowed writes to maxAllowedWrites. 
  // More writes will only give errors when _EEPROMEX_DEBUG is set
  EEPROM.setMaxAllowedWrites(maxAllowedWrites);
  delay(100);
  
  if (TEST_WITH_SERIAL) {
    Serial.begin(9600);
    
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
  }
  
  // obtain last stored values from EEPROM memory after startup
  for (int i = 0; i < COUNT; i++) {
    pinMode(INPUTS[i], INPUT_PULLUP);
    //read in stored data from EEPROM
    codeValue[i] = EEPROM.readLong(i*50);
    codeBits[i] = EEPROM.readByte((i*50)+32);
    codeProtocol[i] = EEPROM.readByte((i*50)+40);
    
    if (TEST_WITH_SERIAL) {
      if (LANG_OUTPUT == LANG_NL) Serial.println("Gevonden commando's in EEPROM:");
      else Serial.println("Found commands in EEPROM:");
      Serial.print(i+1);Serial.print(" ");Serial.print(codeProtocol[i]);
      Serial.print(" 0x");Serial.print(codeValue[i], HEX);
      Serial.print(", bits 0x");Serial.println(codeBits[i], HEX);
    }
  }
  // Program pin is a pushbutton or the pinky
  pinMode(PROGRAM_PIN, INPUT_PULLUP);

  if (TEST_WITH_SERIAL) displayMenu();
  
  if (COUNT > 1) {finger2 = INPUTS[1];}
  if (COUNT > 2) finger3 = INPUTS[2];
  
  ard_effect0_status = -1;
  ard_effect0_start = millis();

  digitalWrite(builtInLed, ! (builtInLed_ON));
  program_state = 0;
}

void displayMenu() {
  Serial.println("GloveIR STEAMbox menu");
  Serial.println("*********************");
  for (int i = 0; i < COUNT; i++) {
    if (LANG_OUTPUT == LANG_NL) {
      Serial.print("Stuur via deze monitor nummer ");
      Serial.print(i+1);
      Serial.print(", om de code voor de ");
      Serial.print(vinger[i]);
      Serial.println(" op te nemen.");
    } else {
      Serial.print("Send via this monitor number ");
      Serial.print(i+1);
      Serial.print(", to record the code for the ");
      Serial.print(vinger_EN[i]);
      Serial.println(".");
    }
  }
  Serial.println();
}
void flickerLED() {
  //Variables of this effect are reffered to with ard_effect0
  boolean restart = false;
  ard_effect0_time = millis() - ard_effect0_start;
  if (ard_effect0_time > EFFECT0_PERIOD) {
    //end effect, make sure it restarts
    if (ard_effect0_status > -1) {
    }
    restart = true;
    ard_effect0_status = -1;
    ard_effect0_start = ard_effect0_start + ard_effect0_time;
    ard_effect0_time = 0;
  }
  if (not restart && ard_effect0_status == -1) {
    ard_effect0_status = 0;
    ard_effect0_start = ard_effect0_start + ard_effect0_time;
    ard_effect0_time = 0;
  digitalWrite(builtInLed, builtInLed_ON);
  }
  if (ard_effect0_time > EFFECT0_1_DURATION && ard_effect0_status < 1) {
   ard_effect0_status = 1;
  digitalWrite(builtInLed, ! (builtInLed_ON));
  }
}

void normalState() {
  if (TEST_WITH_SERIAL) {
    // one can set commands using serial monitor input
    if (Serial.available() > 0) {
      programCode = Serial.parseInt();   // returns 0 on failure/timeout!
      if (programCode >= 1 && programCode <= COUNT ) {
        if (LANG_OUTPUT == LANG_NL) Serial.print("Ontvangen ");
        else Serial.print("Received ");
        Serial.println(programCode);
        programActionForVinger(programCode-1);
        displayMenu();
      }
    }
  }
  
  // react if a button (=closing finger) is pushed
  processInputs();
}

void handleprogrambtnPress() {
  programbtnPressType = programbtnNOPRESS;
      if (digitalRead(programbtn) == programbtn_PRESSED) {
        if (programbtnbuttonActive == false) {
          programbtnbuttonActive = true;
          programbtnbuttonTimer = millis();
        }
        if ((millis() - programbtnbuttonTimer > programbtnlongPressTime) && (programbtnlongPressActive == false)) {
          programbtnlongPressActive = true;
          programbtnPressType = programbtnLONGPRESS;
        }
      } else {
        if (programbtnbuttonActive == true) {
          if (programbtnlongPressActive == true) {
            programbtnlongPressActive = false;
          } else {
            //avoid fast fluctuations to be identified as a click
            if (millis() - programbtnbuttonTimer > programbtnminShortPressTime)
              programbtnPressType = programbtnSHORTPRESS;
          }
          programbtnbuttonActive = false;
        }
      }
}



void handlefinger1Press() {
  finger1PressType = finger1NOPRESS;
      if (digitalRead(finger1) == finger1_PRESSED) {
        if (finger1buttonActive == false) {
          finger1buttonActive = true;
          finger1buttonTimer = millis();
        }
        if ((millis() - finger1buttonTimer > finger1longPressTime) && (finger1longPressActive == false)) {
          finger1longPressActive = true;
          finger1PressType = finger1LONGPRESS;
        }
      } else {
        if (finger1buttonActive == true) {
          if (finger1longPressActive == true) {
            finger1longPressActive = false;
          } else {
            //avoid fast fluctuations to be identified as a click
            if (millis() - finger1buttonTimer > finger1minShortPressTime)
              finger1PressType = finger1SHORTPRESS;
          }
          finger1buttonActive = false;
        }
      }
}



void handlefinger2Press() {
  finger2PressType = finger2NOPRESS;
      if (digitalRead(finger2) == finger2_PRESSED) {
        if (finger2buttonActive == false) {
          finger2buttonActive = true;
          finger2buttonTimer = millis();
        }
        if ((millis() - finger2buttonTimer > finger2longPressTime) && (finger2longPressActive == false)) {
          finger2longPressActive = true;
          finger2PressType = finger2LONGPRESS;
        }
      } else {
        if (finger2buttonActive == true) {
          if (finger2longPressActive == true) {
            finger2longPressActive = false;
          } else {
            //avoid fast fluctuations to be identified as a click
            if (millis() - finger2buttonTimer > finger2minShortPressTime)
              finger2PressType = finger2SHORTPRESS;
          }
          finger2buttonActive = false;
        }
      }
}

void handlefinger3Press() {
  finger3PressType = finger3NOPRESS;
      if (digitalRead(finger3) == finger3_PRESSED) {
        if (finger3buttonActive == false) {
          finger3buttonActive = true;
          finger3buttonTimer = millis();
        }
        if ((millis() - finger3buttonTimer > finger3longPressTime) && (finger3longPressActive == false)) {
          finger3longPressActive = true;
          finger3PressType = finger3LONGPRESS;
        }
      } else {
        if (finger3buttonActive == true) {
          if (finger3longPressActive == true) {
            finger3longPressActive = false;
          } else {
            //avoid fast fluctuations to be identified as a click
            if (millis() - finger3buttonTimer > finger3minShortPressTime)
              finger3PressType = finger3SHORTPRESS;
          }
          finger3buttonActive = false;
        }
      }
}
void programState() {
  handlefinger1Press();

  if (finger1PressType == finger1SHORTPRESS) {
    //START STATEMENTS SHORT PRESS
    program_state = 2;
    //END  STATEMENTS SHORT PRESS
  } else if (finger1PressType == finger1LONGPRESS) {
    //START STATEMENTS LONG PRESS
    //END  STATEMENTS LONG PRESS
  } else if (!finger1longPressActive && digitalRead(finger1) == finger1_PRESSED) {
    //START STATEMENTS PRESS
    //END  STATEMENTS PRESS
  }

  if (COUNT > 1) {
    handlefinger2Press();
  
    if (finger2PressType == finger2SHORTPRESS) {
      //START STATEMENTS SHORT PRESS
      program_state = 3;
      //END  STATEMENTS SHORT PRESS
    } else if (finger2PressType == finger2LONGPRESS) {
      //START STATEMENTS LONG PRESS
      //END  STATEMENTS LONG PRESS
    } else if (!finger2longPressActive && digitalRead(finger2) == finger2_PRESSED) {
      //START STATEMENTS PRESS
      //END  STATEMENTS PRESS
    }
  }


  if (COUNT > 2) {
  handlefinger3Press();
  
    if (finger3PressType == finger3SHORTPRESS) {
      //START STATEMENTS SHORT PRESS
      program_state = 4;
      //END  STATEMENTS SHORT PRESS
    } else if (finger3PressType == finger3LONGPRESS) {
      //START STATEMENTS LONG PRESS
      //END  STATEMENTS LONG PRESS
    } else if (!finger3longPressActive && digitalRead(finger3) == finger3_PRESSED) {
      //START STATEMENTS PRESS
      //END  STATEMENTS PRESS
    }
  }

}

void loop() {
  
  handleprogrambtnPress();

  if (programbtnPressType == programbtnSHORTPRESS) {
    //START STATEMENTS SHORT PRESS
    if (program_state == 0) {
      program_state = 1;
    } else if (program_state == 1) {
      program_state = 0;
    }
    //END  STATEMENTS SHORT PRESS
  } else if (programbtnPressType == programbtnLONGPRESS) {
    //START STATEMENTS LONG PRESS
    //END  STATEMENTS LONG PRESS
  } else if (!programbtnlongPressActive && digitalRead(programbtn) == programbtn_PRESSED) {
    //START STATEMENTS PRESS
    //END  STATEMENTS PRESS
  }

  if (program_state == 0) {
    digitalWrite(builtInLed, ! (builtInLed_ON));
    normalState();
  } else if (program_state == 1) {
    flickerLED();
    programState();
  } else if (program_state == 2) {
    digitalWrite(builtInLed, builtInLed_ON);
    programActionForVinger(0);
    program_state = 0;
  } else if (program_state == 3) {
    digitalWrite(builtInLed, builtInLed_ON);
    programActionForVinger(1);
    program_state = 0;
  } else if (program_state == 4) {
    digitalWrite(builtInLed, builtInLed_ON);
    programActionForVinger(2);
    program_state = 0;
  }
}

void programActionForVinger(int code) {
  // we make us ready to receive signals
  myReceiver.enableIRIn(); // Start the receiver. This uses interrupt code, so we can use delay!
  delay(1000);
  // we write on Serial what user has to do
  if (LANG_OUTPUT == LANG_NL)  Serial.println("Houd de IR-afstandsbediening voor de ontvanger en druk je commando in..");
  else  Serial.println("Hold the IR-remote in front of the receiver and press the button with your command..");
  // loop tot resultaten binnen zijn
  do {
    delay(1000);
    if (LANG_OUTPUT == LANG_NL)  Serial.println("Nog niets ontvangen");
    else Serial.println("Nothing received yet...");
  } while (!myReceiver.getResults());
  // decodeer signaal
  myDecoder.decode();
  // sla signaal op
  storeCode(code);
  if (LANG_OUTPUT == LANG_NL)  Serial.print("Code opgeslagen als signaal: ");
  else Serial.print("Code saved as signal: ");
  Serial.println(code+1);
}


// Stores the code for later playback
void storeCode(int codeIndex) {
  //gotNew = true;    gotOne = true;
  codeProtocol[codeIndex] = myDecoder.protocolNum;
  if (TEST_WITH_SERIAL) {
    if (LANG_OUTPUT == LANG_NL) Serial.print(F("Ontvangen "));
    else Serial.print(F("Received "));
    Serial.print(Pnames(codeProtocol[codeIndex]));
  }
  
  if (codeProtocol[codeIndex] == UNKNOWN) {
    if (TEST_WITH_SERIAL) {
      // print out the data
      if (LANG_OUTPUT == LANG_NL) Serial.println(F(" ruwe data opgelagen."));
      else Serial.println(F(" saving raw data."));
      myDecoder.dumpResults();
    }

    //clean up data as needed
    //The raw time values start in decodeBuffer[1] because
    //the [0] entry is the gap between frames. The address
    //is passed to the raw send routine.
    codeValue[codeIndex] = (uint32_t) & (recvGlobal.decodeBuffer[1]);
    //This isn't really number of bits. It's the number of entries
    //in the buffer.
    codeBits[codeIndex] = recvGlobal.decodeLength - 1;
    //updaten van EEPROM
    EEPROM.updateLong(codeIndex*50, codeValue[codeIndex]);
    EEPROM.updateByte((codeIndex*50)+32, codeBits[codeIndex]);
    EEPROM.updateByte((codeIndex*50)+40, codeProtocol[codeIndex]);
  }
  else {
    if (myDecoder.value == REPEAT_CODE) {
      // Don't record a NEC repeat value as that's useless.
      if (TEST_WITH_SERIAL) {
        if (LANG_OUTPUT == LANG_NL)  Serial.println(F("herhalingscommando; genegeerd."));
        else Serial.println(F("repeat; ignoring."));
      }
    } else {
      codeValue[codeIndex] = myDecoder.value;
      codeBits[codeIndex] = myDecoder.bits;
      //Updaten van EEPROM
      EEPROM.updateLong(codeIndex*50, codeValue[codeIndex]);
      EEPROM.updateByte((codeIndex*50)+32, codeBits[codeIndex]);
      EEPROM.updateByte((codeIndex*50)+40, codeProtocol[codeIndex]);
    }
    if (TEST_WITH_SERIAL) {
      Serial.print(F(" Value:0x"));
      Serial.println(myDecoder.value, HEX);
    }
  }
}


void processInputs() {
  for (int i = 0; i < COUNT; i++) {
    if (digitalRead(INPUTS[i]) == LOW) {
      if (TEST_WITH_SERIAL) {
        if (LANG_OUTPUT == LANG_NL)  Serial.print("Stuur nu code ");
        else Serial.print("Sending out code ");
        Serial.println(i+1);
      }
      sendCode(i);
      delay(100);
    }
  }
}
void sendCode(int codeIndex) {
  // send the raw data we stored!
  mySender.send(codeProtocol[codeIndex], codeValue[codeIndex], codeBits[codeIndex]);
  if (TEST_WITH_SERIAL) {
    if (LANG_OUTPUT == LANG_NL) Serial.print(F("Stuurde "));
    else Serial.print(F("Sent "));
    if (codeProtocol[codeIndex] != UNKNOWN) Serial.print(Pnames(codeProtocol[codeIndex]));
    if (LANG_OUTPUT == LANG_NL) Serial.print(F(" Waarde:0x"));
    else Serial.print(F(" Value:0x"));
    Serial.println(codeValue[codeIndex], HEX);
  }
}
