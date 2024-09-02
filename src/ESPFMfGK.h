// inline guard. Did I mention that c/c++ is broken by design?
#ifndef ESPFMfGK_h
#define ESPFMfGK_h

/*
  ESP32 File Manager for Generation Klick aka ESPFMfGK
    https://github.com/holgerlembke/ESPFMfGK
    lembke@gmail.com

  Changes
    V2.0.4
     + fm.js: langsame Umstellung von "var" auf "let", "use strict";
     + fm.hmtl: "reload file list"

    V2.0.3
     + fm.js: fixed dialog event handling code
     + fm.js: LoadHtmlIncludesProcessor implementiert
     + Arduino-release V2.0.10

    V2.0.2
     + dispFileString auf uint64_t umgestellt

    V2.0.1
     + Fix: https://github.com/holgerlembke/ESPFMfGK/issues/1

    V2.0
     + dauerhafte FensterEditoren

    V1.9
     + new design for "download-all", "rename/move" and "delete" dialog
     + option to remove zipdownloadall-code, saves 4k
     + deactivated lots of serial.prints...
     + Arduino-release V2.0.9

  Changes
    V1.8
     + Arduino-release V2.0.8

  Changes
    V1.8
     + Editorinsert schickt größere Datenchunks

    V1.7
     + preview-fenster-titel transparenz optimiert
     + Redesign ZIP-Schnittstelle, Verlagerung in eigenen .h/.cpp wg. Sourcecodeumfangsverminderung 
     + some sendContent-collector-speedups
     + release as 2.0.2

    V1.6
     + WebPageTitle to set the web page title
     + das Konzept für beliebige Dateisysteme steht
     + nur noch Support für ESP32-Familie
     + Neuer Name: ESP32 File Manager for Generation Klick aka ESPFMfGK
     + Pffff. Geht gut. Alles überarbeiten. Entchaosieren.
     + charset-support, utf-8 scheint zu funktionieren
     + Preview. Yeah.
     + Rename works across folder structures (ok, it is copy+delete)
     + create empty new file
     + deflate datafiles. 8k instead of 33k
     + release as 2.0.1 

    V1.5
     x starting rework for version 2.0
     x remove all spiffs stuff
     x the big rename to ...2...
     + Entwurf der Callback-Schnittstelle für Flags
     x das File-Insert ist deutlich komplexer und überträgt weniger Daten
     x more fancy web gui: Prozent-Anzeigen! (Mainly because SD is so slow...)
     x implemented a basic concept to dynamically add html/css-content and handle URL clicks

   V1.x formerly known as ESPxWebFlMgr     
*/

#include <Arduino.h>
#include <inttypes.h>

#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>

/* Undefine both of these to save about 8k to 33k code space.
     Now you have to put the files from "<library>/filemanager" into a FS.
     The FS is indexed by FileSystemIndexForWebPages and follows the order used by AddFS().
     So by default the /fm.* are put on the first added FS, that one, that is shown in the browser by default as the first FS.
     You can use the callback to hide the /fm.* files, isFileManagerInternalFile(String fn) helps with that.

     Difference:
       fileManagerServerStaticsInternallyDeflate is compressed and a tiny little bit violation protocol
          by allways serving deflate files, whatever the browsers asks for...
       fileManagerServerStaticsInternally serves pure uncompressed data. Huge. 33k.

     For compatibility reasons, the fileManagerServerStaticsInternally is activated by default.
*/
#define fileManagerServerStaticsInternally
// #define fileManagerServerStaticsInternallyDeflate


// if you do not need the "download all files" function, commenting out this define saves about 4k code space
#define ZipDownloadAll


// Callback for "foreign" URLs called to web server. If not set, all files will be served
//  Result:
//      0: deny
//      1: allow and serve via file system, this is the default action without this callback
//      2: send back the message in data
// Main usage of this callback is to implement hyperlinks in in additional html content added via ExtraHTML*
typedef int (*ESPxWebCallbackURL_t)(String &data);

// Callback for checking file flags. Please look into the examples.
typedef uint32_t (*ESPxWebCallbackFlags_t)(fs::FS &fs, String filename, uint32_t flags);

// Callback for the HtmlIncludes
typedef bool (*HtmlIncludesCallback_t)(WebServer *webserver);

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
  // beim Umbenennen
  const static uint32_t flagIsValidTargetFilename =  1 << 9;
  // beim Überprüfen, ob eine Dateisystem-Aktion zulässig ist
  const static uint32_t flagIsValidAction =  1 << 10;
  // allowed to create new files
  const static uint32_t flagCanCreateNew =  1 << 11;

  ESPxWebCallbackFlags_t checkFileFlags = NULL;
  ESPxWebCallbackURL_t checkURLs = NULL;
  HtmlIncludesCallback_t HtmlIncludesCallback = NULL;

  // Frage aller Fragen: sollte eine automatische Umschaltung Flat/Treeview gebaut werden und
  //                     wie wäre dann der Ablauf
  enum DefaultViewMode_t { dvmNone, dvmFlat, dvmTree };

private:
  struct FileSystemInfo_t  // sizeof: 24, packed: 21
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
  String dispFileString(uint64_t fs, bool printorg);
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
  void servefile(String uri);
  void HtmlIncludesInterface(void);
  void Illegal404();

  // Hlpr für die Verwaltung, was angezeigt werden soll
  static const int8_t maxfilesystems = 4;                             // !!!!!!!
  FileSystemInfo_t fsinfo[maxfilesystems];
  int maxfilesystem = 0;
  int lastFileSystemIndex = -1;
  int getFileSystemIndex(bool uselastFileSystemIndex = true);
  bool ShowInTreeView();
  String CurrentPath();

  // Dateinamen mit dem Format IDX:/path/path/fn
  int getFSidxfromFilename(String fn);   // -> IDX bzw. -1
  String getCleanFilename(String fn);    // -> immer /path/path/fn

  String pathname(String fn);
  bool CopyMoveFile(String oldfilename, String newfilename, bool move);
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
  // add this html-files as windowed item, can be list ;-separated
  String HtmlIncludes = "";
};

#endif
