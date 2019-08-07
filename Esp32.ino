/*
  SD card test

 This example shows how use the utility libraries on which the'
 SD library is based in order to get info about your SD card.
 Very useful for testing a card when you're not sure whether its working or not.

 The circuit:
  * SD card attached to SPI bus as follows:
 ** MOSI - pin 11 on Arduino Uno/Duemilanove/Diecimila
 ** MISO - pin 12 on Arduino Uno/Duemilanove/Diecimila
 ** CLK - pin 13 on Arduino Uno/Duemilanove/Diecimila
 ** CS - depends on your SD card shield or module.
     Pin 4 used here for consistency with other Arduino examples


 created  28 Mar 2011
 by Limor Fried
 modified 9 Apr 2012
 by Tom Igoe
 */
// include the SD library:
#include <SPI.h>
#include <SD.h>
#include <ctype.h>

#define RXD2 16
#define TXD2 17

// Define CS pin for the SD card module
#define SD_CS 5

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RXD2, TXD2);
  while (!Serial1) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Power on your FPGA...");

  byte b;
  do {
    if (Serial1.available()) {
      b = Serial1.read();   
      Serial.println(b);
    }
  } while (b != 77); // 'M' character
  
  byte buff[10];
  int i = 0;
  do {
    if (Serial1.available()) {
      b = Serial1.read();   
      Serial.println(b);
      buff[i++] = b;
    }
  } while (i < 5); // followed by 13, 10, 3, 3, 3
  
  if (buff[0] == 13 && buff[1] == 10 && buff[2] == 3) {
    Serial.println("FPGA is connected.");
  
    Serial.println("\nInitializing SD card...");
    
    // Initialize SD card
    SD.begin(SD_CS);  
    if(!SD.begin(SD_CS)) {
      Serial.println("Card Mount Failed");
      return;
    }
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE) {
      Serial.println("No SD card attached");
      return;
    }
    Serial.println("Initializing SD card...");
    if (!SD.begin(SD_CS)) {
      Serial.println("ERROR - SD card initialization failed!");
      return;    // init failed
    }

   File root;
   root = SD.open("/");
   File boot = findBootFile(root);
   if (&boot != &root) { 
     Serial.print("Found boot file: "); 
     Serial.println(boot.name()); 
     // send file size
     int size = boot.size();
     Serial.print("Boot file size:");
     Serial.println(size, DEC);
     size -= 0xB000;
     long size1 = size;
     Serial.print("Boot image size:");
     Serial.println(size, DEC);
     // send image size, byte by byte
     b = size & 255;
     Serial1.write(b);
     Serial.println(b, HEX);
     delay(100);
     size >>= 8;
     b = size & 255;
     Serial1.write(b);
     Serial.println(b, HEX);
     delay(100);
     // last two bytes are 0
     b = 0;
     Serial1.write(b);
     delay(100);
     Serial1.write(b);
     delay(100);
     // read back size
     i = 0;
     do {
      if (Serial1.available()) {
        b = Serial1.read();   
        Serial.println(b, HEX);
        buff[i++] = b;
      }
    } while (i < 2);
    int size_r = 0;
    size_r = buff[0] + buff[1] * 256;
    Serial.print("Returned size: ");
    Serial.println(size_r, DEC); 
    if (size_r == size1) {
      File myFile = SD.open(boot.name());
      if (myFile) {
        myFile.seek(0xB000);
        // read from the file until there's nothing else in it:
        while (myFile.available()) {
          Serial1.write(myFile.read());
        }
        // close the file:
        myFile.close();

        i = 0;
        do {
          if (Serial1.available()) {
            b = Serial1.read();   
            Serial.println(b, HEX);
            buff[i++] = b;
          }
        } while (i < 2);
        int chk_r = buff[0] + buff[1] * 256;
        Serial.print("Received checksum: ");
        Serial.println(chk_r);
      } else {
        // if the file didn't open, print an error:
        Serial.println("Error opening boot image!");
      }
    } else {
      Serial.println("Sent and received size mismatch!");
    }
   }
 } else {
  Serial.println("No FPGA detected.");
 }
}

File findBootFile(File dir) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    if (!entry.isDirectory()) {
      // files have sizes, directories do not
      Serial.print(entry.name());
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
      char upper[100];
      strcpy(upper, entry.name());
      for(char* c=upper; *c=toupper(*c); ++c);    
      
      if (strcmp(upper, "/BOOT.BIN") == 0) {
        Serial.print("FOUND BOOT.BIN! Size: ");
        Serial.println(entry.size(), DEC);
        return entry;
      }
    }
    entry.close();
  }
  return dir;
}

void loop(void) {

}
