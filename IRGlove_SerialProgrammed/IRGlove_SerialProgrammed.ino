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

// set serial output on to test with it
#define TEST_WITH_SERIAL true

/*  CODE 
 *   
*/

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

void loop() {
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
