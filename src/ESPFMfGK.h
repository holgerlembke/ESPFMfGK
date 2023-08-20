// inline guard. Did I mention that c/c++ is broken by design?
#ifndef ESPFMfGK_h
#define ESPFMfGK_h

/*alt-shift-f

  ESP32 File Manager for Generation Klick ESPFMfGK
    https://github.com/holgerlembke/ESPFMfGK
    lembke@gmail.com

  neue features: preview panel

  ++ alles mit fs:SD testen
  ++ gzipper testen
  ++ copy file from one fs to another fs/another folder

  ### ggf. braucht es eine dateinamen-codierung -> webseite, um alle spezialfälle sauber zu transportieren

  getrennten eintrag für das fs, wo die webseiten liegen

  Changes
    V1.6
     + WebPageTitle to set the pweb page title
     + das Konzept für beliebige Dateisysteme steht
     + nur noch Support für ESP32-Familie
     + Neuer Name: ESP32 File Manager for Generation Klick aka ESPFMfGK
     + Pffff. Geht gut. Alles überarbeiten. Entchaosieren.
     + charset-support, utf-8 scheint zu funktionieren

    V1.5
     x starting rework for version 2.0
     x remove all spiffs stuff
     x the big rename to ...2...
     + Entwurf der Callback-Schnittstelle für Flags
     x das File-Insert ist deutlich komplexer und überträgt weniger Daten
     x more fancy web gui: Prozent-Anzeigen! (Mainly because SD is so slow...)
     x implemented a basic concept to dynamically add html/css-content and handle URL clicks

    V1.03
     x removed all SPIFFS from ESP32 version, switched fully to LittleFS
     x fixed rename+delete for ESP32+LittleFS (added "/")

    V1.02
     x fixed the way to select the file system by conditional defines

    V1.01
     + added file name progress while uploading
     x fixed error in ZIP file structure (zip.bitflags needs a flag)

    V1.00
     + out of V0.9998...
     + ESP8266: LittleFS is default
     + javascript: added "msgline();"
     + javascript: added "Loading..." as a not-working-hint to show that Javascript is disabled
     + cleaning up the "/"-stuff (from SPIFF with leading "/" to LittleFS without...)
     + Warning: esp8266 2.7.4 has an error in mime::getContentType(path) for .TXT. Fix line 65 is { kTxtSuffix, kTxt },
     + review of "edit file", moved some stuff to ESPxWebFlMgrWpF.h

    V0.9998
     + Renamed to ESPxWebFlMgr and made it work with esp32 and esp8266
     + separated file manager web page, "build script" to generate it

    V0.7
     + "Download all files" creates a zip file from all files and downloads it
     + option to set background color
     - html5 fixes

    V0.6
     + Public Release on https://github.com/holgerlembke/ESP8266WebFlMgr
*/

#include <Arduino.h>
#include <inttypes.h>

#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>

/* Undefine this to save about 10k code space.
     Now you have to put the files from "<library>/filemanager" into a FS.
     The FS is indexed by FileSystemIndexForWebPages and follows the order used by AddFS().
     So by default the /fm.* are put on the first added FS, that one, that is shown in the browser by default as the first FS.
     You can use the callback to hide the /fm.* files, isFileManagerInternalFile(String fn) helps with that.
*/
// #define fileManagerServerStaticsInternally


// Callback for "foreign" URLs called to web server. If not set, all files will be served
//  Result:
//      0: deny
//      1: allow and serve via file system, this is the default action without this callback
//      2: send back the message in data
// Main usage of this callback is to implement hyperlinks in in additional html content added via ExtraHTML*
typedef int (*ESPxWebCallbackURL_t)(String &data);

// Callback for checking file flags. Please look into the examples.
typedef uint32_t (*ESPxWebCallbackFlags_t)(fs::FS &fs, String filename);

class ESPFMfGK
{
public:
  // Flags, sync with fm.js, this has some room to grow, 32 bits ought to be enough for anybody
  const static uint32_t flagCanDelete = 1 << 0;
  const static uint32_t flagCanRename = 1 << 1;
  // see CanUpload, Edits "save" button will fail if not set
  const static uint32_t flagCanEdit = 1 << 2;
  // Allowed to be previewed. Browser does the preview, so it depends on that.
  const static uint32_t flagAllowPreview = 1 << 3;
  const static uint32_t flagCanGZip = 1 << 4;
  const static uint32_t flagCanDownload = 1 << 5;
  const static uint32_t flagAllowInZip = 1 << 6;
  // A file with this name can be uploaded. An upload wont work if this flag is not set!
  const static uint32_t flagCanUpload = 1 << 7;
  // File will not be shown at all
  const static uint32_t flagIsNotVisible = 1 << 8;

  ESPxWebCallbackFlags_t checkFileFlags = NULL;
  ESPxWebCallbackURL_t checkURLs = NULL;

private:
  struct FileSystemInfo_t
  {
    String fsname;
    bool AutoTreemode;
    fs::FS *filesystem;
  };

  word _Port;
  WebServer *fileManager = NULL;
  File fsUploadFile;
  String _backgroundColor = "black";

  void fileManagerNotFound(void);
  String dispIntDotted(size_t i);
  String dispFileString(size_t fs, bool printorg);
  String CheckFileNameLengthLimit(String fn);

  // the webpage
  void fileManagerIndexpage(void);
  void fileManagerJS(void);
  void fileManagerCSS(void);
  void fileManagerGetBackGround(void);

  // javascript xmlhttp includes
  String colorline(int i);
  String escapeHTMLcontent(String html);
  void fileManagerFileListInsert(void);
  boolean allowAccessToThisFile(const String filename);
  void fileManagerReceiverOK(void);
  void fileManagerReceiver(void);

  // Build file Index insert
  bool gzipperexists;
  String Folder1LevelUp(String foldername);
  void recurseFolderList(String foldername, int maxtiefe, int tiefe);
  void recurseFolder(String foldername, bool flatview, int maxtiefe, bool iststart, int &linecounter);
  String getFileNameFromParam(uint32_t flag);

  // central job processor
  void fileManagerJobber(void);
  void fileManagerBootinfo(void);
  void fileManagerFileEditorInsert(String &filename);
  void fileManagerDownload(String &filename);
  void servefile(String uri, int overridefs = -1);
  void Illegal404();

  // Zip-File uncompressed/stored
  void getAllFilesInOneZIP(void);
  int WriteChunk(const char *b, size_t l);

  // Hlpr für die Verwaltung, was angezeigt werden soll
  static const int8_t maxfilesystems = 3;                             // !!!!!!!
  FileSystemInfo_t fsinfo[maxfilesystems];
  int maxfilesystem = 0;
  int lastFileSystemIndex = -1;
  int getFileSystemIndex(bool uselastFileSystemIndex = true);
  bool ShowInTreeView();
  String CurrentPath();

  String DeUmlautFilename(String fn);
  // total/used are not exposed in FS::FS. Who knows why.
  uint64_t totalBytes(fs::FS *fs);
  uint64_t usedBytes(fs::FS *fs);

  // Flags für Datenkommunikation
  String itemtrenner = "\x02\x01\x04";
  String beginoffiles  = "\x03\x01\x02";
  String antworttrenner = "\x02\x01\x03";
  String extrabootinfotrenner = "\x02\x01\x07";

  String svi = "/System Volume Information";

public:
  ESPFMfGK(word port);
  virtual ~ESPFMfGK();

  bool begin();
  void end();
  virtual void handleClient();

  bool AddFS(fs::FS &fs, String FSname, bool AutoTreemode);

  // by default, ESPFMfGK will look for the /fm.* stuff at this file system
  int FileSystemIndexForWebPages = 0;
  bool isFileManagerInternalFile(String fn);

  // must be a valid css color name, see https://en.wikipedia.org/wiki/Web_colors
  String BackgroundColor = "";
  // additional html inserted into the foot below the web page
  String ExtraHTMLfoot = "";
  String WebPageTitle = "";
  // set "accept-charset=\"utf-8\"" for utf-8 support in textarea
  String textareaCharset = "";
};

#endif
