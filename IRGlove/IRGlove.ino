#include <EEPROMex.h>
#include <EEPROMVar.h>
/* record.ino Example sketch for IRLib2
    Illustrate how to record a signal and then play it back.
*/
#include <IRLibDecodeBase.h>  //We need both the coding and
#include <IRLibSendBase.h>    // sending base classes
// we don't need to understand what is send, only be able to reproduce it!
//#include <IRLib_P01_NEC.h>    //Lowest numbered protocol 1st
//#include <IRLib_P02_Sony.h>   // Include only protocols you want
//#include <IRLib_P03_RC5.h>
//#include <IRLib_P04_RC6.h>
//#include <IRLib_P05_Panasonic_Old.h>
//#include <IRLib_P07_NECx.h>
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
IRrecv myReceiver(12); //pin number for the receiver

const int COUNT = 3;
int programCode = 0;
const String vinger[COUNT] = {"wijsvinger", "middelvinger", "ringvinger"};

const int INPUTS[COUNT] = {8, 9, 10};
const int PROGRAM_PIN = 7;

// Storage for the recorded code
uint8_t codeProtocol;  // The type of code
uint32_t codeValue[COUNT] = {0}; // The data bits if type is not raw
uint8_t codeBits[COUNT] = {0};      // The length of the code in bits

//These flags keep track of whether we received the first code
//and if we have have received a new different code from a previous one.
bool gotOne, gotNew;

void setup() {
  //gotOne = false; gotNew = false;
  codeProtocol = UNKNOWN;
  //codeValue = 0;
  //EEPROM_readAnything(0, configuration);

  Serial.begin(9600);
  // obtain last stored values from EEPROM memory after startup
  for (int i = 0; i < COUNT; i++) {
    pinMode(INPUTS[i], INPUT_PULLUP);
    //Inlezen van EEPROM
    codeValue[i] = EEPROM.readLong(i*50);
    codeBits[i] = EEPROM.readByte((i*50)+40);
  }
  // Program pin is a pushbutton or the pinky
  pinMode(PROGRAM_PIN, INPUT_PULLUP);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  displayMenu();
}

void displayMenu() {
  Serial.println("GloveIR STEAMbox menu");
  Serial.println("*********************");
  for (int i = 0; i < COUNT; i++) {
    Serial.print("Stuur via deze monitor nummer ");
    Serial.print(i+1);
    Serial.print(", om de code voor de ");
    Serial.print(vinger[i]);
    Serial.println(" op te nemen.");
  }
  Serial.println();
}

// Stores the code for later playback
void storeCode(int codeIndex) {
  //gotNew = true;    gotOne = true;
  codeProtocol = myDecoder.protocolNum;
  Serial.print(F("Received "));
  Serial.print(Pnames(codeProtocol));
  
  if (codeProtocol == UNKNOWN) {
    // print out the data
    Serial.println(F(" saving raw data."));
    myDecoder.dumpResults();

    //clean up data as needed
    //The raw time values start in decodeBuffer[1] because
    //the [0] entry is the gap between frames. The address
    //is passed to the raw send routine.
    codeValue[codeIndex] = (uint32_t) & (recvGlobal.decodeBuffer[1]);
    //This isn't really number of bits. It's the number of entries
    //in the buffer.
    codeBits[codeIndex] = recvGlobal.decodeLength - 1;
//    codeValue[codeIndex] = myDecoder.value;
//    codeBits[codeIndex] = myDecoder.bits;
    //updaten van EEPROM
    EEPROM.updateLong(codeIndex*50, codeValue[codeIndex]);
    EEPROM.updateByte((codeIndex*50)+40, codeBits[codeIndex]);
  }
  else {
    if (myDecoder.value == REPEAT_CODE) {
      // Don't record a NEC repeat value as that's useless.
      Serial.println(F("repeat; ignoring."));
    } else {
      codeValue[codeIndex] = myDecoder.value;
      codeBits[codeIndex] = myDecoder.bits;
      //Updaten van EEPROM
      EEPROM.updateLong(codeIndex*50, codeValue[codeIndex]);
      EEPROM.updateByte((codeIndex*50)+40, codeBits[codeIndex]);
    }
    Serial.print(F(" Value:0x"));
    Serial.println(codeValue[codeIndex], HEX);
  }
}

void sendCode(int codeIndex) {
//  if ( !gotNew ) { //We've already sent this so handle toggle bits
//    if (codeProtocol == RC5) {
//      codeValue[codeIndex] ^= 0x0800;
//    }
//    else if (codeProtocol == RC6) {
//      switch (codeBits[codeIndex]) {
//        case 20: codeValue[codeIndex] ^= 0x10000; break;
//        case 24: codeValue[codeIndex] ^= 0x100000; break;
//        case 28: codeValue[codeIndex] ^= 0x1000000; break;
//        case 32: codeValue[codeIndex] ^= 0x8000; break;
//      }
//    }
//  }
//  gotNew = false;
//  if (codeProtocol == UNKNOWN) {
//    //The raw time values start in decodeBuffer[1] because
//    //the [0] entry is the gap between frames. The address
//    //is passed to the raw send routine.
//    codeValue[codeIndex] = (uint32_t) & (recvGlobal.decodeBuffer[1]);
//    //This isn't really number of bits. It's the number of entries
//    //in the buffer.
//    codeBits[codeIndex] = recvGlobal.decodeLength - 1;
//    Serial.println(F("Sent raw"));
//  }
  // send the raw data we stored!
  mySender.send(codeProtocol, codeValue[codeIndex], codeBits[codeIndex]);
  if (codeProtocol == UNKNOWN) return;
  Serial.print(F("Sent "));
  Serial.print(Pnames(codeProtocol));
  Serial.print(F(" Value:0x"));
  Serial.println(codeValue[codeIndex], HEX);
}

void loop() {
  if (Serial.available() > 0) {
    programCode = Serial.parseInt();   // returns 0 on failure/timeout!
    if (programCode >= 1 && programCode <= COUNT ) {
      Serial.print("Received ");
      Serial.println(programCode);
      procesCommand(programCode-1);
      displayMenu();
    }
  }
  // react if a button (=closing finger) is pushed
  procesInputs();
}

void procesCommand(int code) {
  // we make us ready to receive signals
  myReceiver.enableIRIn(); // Start the receiver. This uses interrupt code, so we can use delay!
  delay(1000);
  // we write on Serial what user has to do
  Serial.println("Houd de IR-afstandsbediening voor de ontvanger en druk je commando in..");
  // loop tot resultaten binnen zijn
  do {
    delay(1000);
    Serial.println("Nog niets ontvangen");
  } while (!myReceiver.getResults());
  // decodeer signaal
  myDecoder.decode();
  // sla signaal op
  storeCode(code);
  Serial.print("Code opgeslagen als signaal: ");
  Serial.println(code+1);
}

void procesInputs() {
  for (int i = 0; i < COUNT; i++) {
    if (digitalRead(INPUTS[i]) == LOW) {
      Serial.print("Sending out code ");
      Serial.println(i+1);
      sendCode(i);
      delay(100);
    }
  }
}
