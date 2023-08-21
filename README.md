# ESPFMfGK
ESP32 File Manager for Generation Klick

Ongoing work



# File menu functions

## Delete

Just what it says: it deletes the file. Currently no way to delete folders, but they are not shown if empty.

## Rename/Move

This will move a file from one location to another. It creates the target folder. It works across devices.

File name syntax can be: 

* /path/path/filename 
* deviceindex:/path/path/filename

Deviceindex follows the order of ESPFMfGK::AddFS calls, 0...n-1.

## Edit

Lets edit a file! Works well for everything that is editable.

## Preview

Opens a preview window for the file. It depends on what the browser can display, so everything editable and JPEG/PNG files should work. The windows existance is disconnected from the files existance, it is a snapshot.





# Stuff I used

* CRC32 from https://github.com/bakercp/CRC32
* gzip-js from https://www.npmjs.com/package/gzip-js
* browserify from http://browserify.org/
* Arduino core for ESP8266 WiFi chip from https://github.com/esp8266/Arduino
* Arduino core for ESP32 WiFi chip from https://github.com/espressif/arduino-esp32
* Infinidash certification NDA toolkit
