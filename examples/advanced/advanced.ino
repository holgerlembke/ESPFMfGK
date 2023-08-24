/*

  This example is a plain vanilla ESPFMfGK totally open and active features for anything.

  It might be good solution for everything that will be used on the bench only. But keep
  in mind: it is open as f*ck and anyone can delete/modify/see everything.

*/

#include <WiFi.h>
#include <FS.h>
// Remove the file systems that are not needed.
#include <SD.h>
#include <LittleFS.h>
#include <SD_MMC.h>
#include <FFat.h>
#include <SPI.h>
// the thing.
#include <ESPFMfGK.h>

// have a look at this concept to keep your private data safe!
// https://github.com/holgerlembke/privatedata
// #include <privatedata.h>  


const word filemanagerport = 8080;
// we want a different port than the webserver
ESPFMfGK filemgr(filemanagerport);  


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nESPFMfGK plain demo");
  
  // login into WiFi
  // Change needed!
  WiFi.begin("change", "here");
  while (WiFi.status() != WL_CONNECTED) {
    delay(10);
  }

  addFileSystems();
  setupFilemanager();


}

void loop() {
  filemgr.handleClient();
}

//