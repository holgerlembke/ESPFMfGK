# ESPFMfGK

ESP32 File Manager for Generation Klick

image

ESPFMfGK is a simple to use web interface that allows you to upload files with drag and drop, download files, edit files, move files and much more within your ESP32 file space. It supports all file systems (FFAT, SD, SD-MMC, LittleFS, SPIFFS) and an unlimited number of devices all at the same time. 

ESPFMfGK is the successor of Award Winning https://github.com/holgerlembke/ESPxWebFlMgr.

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

### Edit

Lets edit a file! Works well for everything that is editable.

### Preview

Opens a preview window for the file. It depends on what the browser can display, so everything editable and JPEG/PNG files should work. The windows existance is disconnected from the files existance, it is a snapshot.

### Download all files

Broken. Work for later.

# Usage

Look into the examples. They are a good boiler plate to start from.

They explain the security system around managing files visibility and operations. In most cases it will be a simple copy-paste-use thingy. 

# define fileManagerServerStaticsInternally

By design concept ESPFMfGK is inteded as "drop it in and that is it" solution. To archive that it includes its
own
html/css/javascript files. These are stored in code space and eat something around 35k.

If you want to reduce the code footprint, undefine that define and put those files into the first added filesystem. No free lunch, either code space or file system.


# Stuff I used

* CRC32 from https://github.com/bakercp/CRC32
* gzip-js from https://www.npmjs.com/package/gzip-js
* browserify from http://browserify.org/
* Arduino core for ESP8266 WiFi chip from https://github.com/esp8266/Arduino
* Arduino core for ESP32 WiFi chip from https://github.com/espressif/arduino-esp32
* Infinidash certification NDA toolkit
