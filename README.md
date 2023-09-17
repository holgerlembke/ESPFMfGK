# ESPFMfGK

ESP32 File Manager for Generation Klick

![this is it](https://raw.githubusercontent.com/holgerlembke/ESPFMfGK/main/img/bild1.jpg)


ESPFMfGK is a simple to use web interface that allows you to upload files with drag and drop, download files, edit files, move files and much more within your ESP32 file space. It supports all file systems (FFAT, SD, SD-MMC, LittleFS, SPIFFS) and an unlimited number of devices all at the same time. 

![this is it](https://raw.githubusercontent.com/holgerlembke/ESPFMfGK/main/img/bild2.jpg)

![this is it](https://raw.githubusercontent.com/holgerlembke/ESPFMfGK/main/img/bild3.jpg)

ESPFMfGK is the successor of Award Winning https://github.com/holgerlembke/ESPxWebFlMgr.

![this is it](https://raw.githubusercontent.com/holgerlembke/ESPFMfGK/main/img/bild4.jpg)

It has a one-the-device file editor. And a persistent preview function.

# File menu functions

### Download

Click on the filename. File is downloaded. That simple.

### Delete

Just what it says: it deletes the file. Currently no way to delete folders, but they are not shown if empty.

### Rename/Move

This will move a file from one location to another. It creates the target folder. It works across devices.

File name syntax can be: 

* /path/path/filename 
* deviceindex:/path/path/filename

Deviceindex follows the order of ESPFMfGK::AddFS calls, 0...n-1.

Above screenshot shows an example: FFat is device 0, SD-Card is device 1. To move a file from FFAT to SD-Card
simply click on (R)ename and add a 1: at the start of the filename.

### Edit

Permanent editor windows! Just open a file in a windowed editor, change content, click "save", change content, click "save". Works well for everything that is editable.

### Preview

Opens a preview window for the file. It depends on what the browser can display, so everything editable and JPEG/PNG files should work. The windows existance is disconnected from the files existance, it is a snapshot.

### Download all files

Easiest way to get all the log files from any device: Download all files from a device. Or all files from a folder including its sub-folders. Into one big ZIP file. 

# Usage

Look into the examples. They are a good boiler plate to start from.

They explain the security system around managing files visibility and operations. In most cases it will be a simple copy-paste-use thingy. 

# Defines

Defines are used to configure some basic features of ESPFMfGK.

## define fileManagerServerStaticsInternally

By design concept ESPFMfGK is inteded as "drop it in and that is it" solution. To archive that it includes 
its own html/css/javascript files. They are stored in code space and eat something around 35k.

If you want to reduce the code footprint, undefine that define and put the files from "filemanager" folder into the first added filesystem. No free lunch, either code space or file system.

## define fileManagerServerStaticsInternallyDeflate

After reading that stuff in the chapter above there is an alternative version of storing the html/css/javascript files in code space: deflated data. That reduces the footprint to about 10k.

It is a tiny little bit http-protocol incompatible by ignoring whatever content the browser requests and always sends the deflated data.

So either define one of those defines or none.

(All numbers are ballpark figures only and generous rounded up to include future code expansions.)

## ZipDownloadAll

Comment this define if you do not need the "download all files" functionality. It will save about 4 to 5k code space.


# Stuff I used

* CRC32 from https://github.com/bakercp/CRC32
* gzip-js from https://www.npmjs.com/package/gzip-js
* browserify from http://browserify.org/
* Arduino core for ESP8266 WiFi chip from https://github.com/esp8266/Arduino
* Arduino core for ESP32 WiFi chip from https://github.com/espressif/arduino-esp32
* Infinidash certification NDA toolkit
