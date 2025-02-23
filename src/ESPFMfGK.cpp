#include <Arduino.h>
#include <inttypes.h>
#include <ESPFMfGK.h>
#include <ESPFMfGKWp.h>        // web page/javascript/css
#include <ESPFMfGKWpDeflate.h> // web page/javascript/css
#include <ESPFMfGKWpF2.h>      // some form fragments
#include <ESPFMfGKGa.h>        // Den ZIP-Code

#include <WebServer.h>
#include <FS.h>
#include <SD.h>
#include <LittleFS.h>
#include <SD_MMC.h>
#include <FFat.h>
#include <detail/mimetable.h>


// copy of static String getContentType(const String &path) from detail/RequestHandlersImpl.h
// decorated with some "mime::"
String getContentType(const String &path)
{
    char buff[sizeof(mime::mimeTable[0].mimeType)];

    // Check all entries but last one for match, return if found
    for (size_t i = 0; i < sizeof(mime::mimeTable) / sizeof(mime::mimeTable[0]) - 1; i++) {
      strcpy_P(buff, mime::mimeTable[i].endsWith);
      if (path.endsWith(buff)) {
        strcpy_P(buff, mime::mimeTable[i].mimeType);
        return String(buff);
      }
    }
    // Fall-through and just return default type
    strcpy_P(buff, mime::mimeTable[sizeof(mime::mimeTable) / sizeof(mime::mimeTable[0]) - 1].mimeType);
    return String(buff);
}

//*****************************************************************************************************
ESPFMfGK::ESPFMfGK(word port)
{
  _Port = port;
  // init everything else, just to be sure...
  end();
}

//*****************************************************************************************************
ESPFMfGK::~ESPFMfGK()
{
  end();
}

//*****************************************************************************************************
bool ESPFMfGK::begin()
{
  if (fsinfo[0].filesystem == NULL)
  {
    Serial.println("Panic: FileManager not started. No filesystem added.");
    return false;
  }

#ifdef fileManagerServerStaticsInternallyDeflate
  Serial.println("Flag: fileManagerServerStaticsInternallyDeflate");
#endif

#ifdef fileManagerServerStaticsInternally
  Serial.println("Flag: fileManagerServerStaticsInternally");
#endif

  fileManager = new WebServer(_Port);

#ifdef fileManagerServerStaticsInternally
  fileManager->on("/", HTTP_GET, std::bind(&ESPFMfGK::fileManagerIndexpage, this));
  fileManager->on("/fm.css", HTTP_GET, std::bind(&ESPFMfGK::fileManagerCSS, this));
  fileManager->on("/fm.js", HTTP_GET, std::bind(&ESPFMfGK::fileManagerJS, this));
#endif
  // handles the file list insert
  fileManager->on("/i", HTTP_GET, std::bind(&ESPFMfGK::fileManagerFileListInsert, this));
  // handles the boot info (color, additional hmtl etc.
  fileManager->on("/b", HTTP_GET, std::bind(&ESPFMfGK::fileManagerBootinfo, this));
  // processes all the requests (edit,delete,rename, etc)
  fileManager->on("/job", HTTP_GET, std::bind(&ESPFMfGK::fileManagerJobber, this));
  // Interface Calls from HtmlIncludes
  fileManager->on("/if", HTTP_GET, std::bind(&ESPFMfGK::HtmlIncludesInterface, this));
  // file receiver with attached file to form
  fileManager->on("/r", HTTP_POST, std::bind(&ESPFMfGK::fileManagerReceiverOK, this),
                  std::bind(&ESPFMfGK::fileManagerReceiver, this));

  fileManager->onNotFound(std::bind(&ESPFMfGK::fileManagerNotFound, this));

  fileManager->begin();

  return true;
}

//*****************************************************************************************************
void ESPFMfGK::end()
{
  if (fileManager)
  {
    delete fileManager;
    fileManager = NULL;
  }
  for (int i = 0; i < 2; i++)
  {
    fsinfo[i].filesystem = NULL;
    fsinfo[i].fsname = "";
  }
}

//*****************************************************************************************************
bool ESPFMfGK::AddFS(fs::FS &fs, String FSname, bool AutoTreemode)
{
  // Add into first slot, if empty. Else always add into second.
  if (maxfilesystem < maxfilesystems)
  {
    fsinfo[maxfilesystem].filesystem = &fs;
    fsinfo[maxfilesystem].fsname = FSname;
    fsinfo[maxfilesystem].AutoTreemode = AutoTreemode;
    maxfilesystem++;
    return true;
  }
  return false;
}

//*****************************************************************************************************
void ESPFMfGK::handleClient()
{
  if (fileManager)
  {
    fileManager->handleClient();
  }
}

//*****************************************************************************************************
// privates start here
//*****************************************************************************************************
//*****************************************************************************************************
void ESPFMfGK::servefile(String uri)
{
  // Handle the servinf of the "fm.*"-files
  int fsi = getFileSystemIndex(false);
  if (fsi == -1)
  {
    fsi = 0;
  }
  /** /
  Serial.print(F("File system id: "));
  Serial.println(fsi);
  /**/

  String contentTyp = getContentType(uri);

  if (fsinfo[fsi].filesystem->exists(uri))
  {
    File f = fsinfo[fsi].filesystem->open(uri, "r");
    if (f)
    {
      if (fileManager->streamFile(f, contentTyp) != f.size())
      {
        // Serial.println(F("Sent less data than expected!"));
        // We should panic a little bit.
      }
      f.close();
      return;
    }
  }

  fileManager->send(404, F("text/plain"), F("File not found, URI invalid."));
}

//*****************************************************************************************************
//*****************************************************************************************************
bool ESPFMfGK::isFileManagerInternalFile(String fn)
{
  fn.toLowerCase();
  if ((fn == "/fm.html") || (fn == "/fm.css") || (fn == "/fm.js"))
  {
    return true;
  }
  else
  {
    return false;
  }
}

//*****************************************************************************************************
//*****************************************************************************************************
void ESPFMfGK::fileManagerNotFound(void)
{
  String uri = fileManager->uri();

#ifdef fileManagerServerStaticsInternallyDeflate
  if ((uri == "/fm.html") || (uri == "/"))
  {
    //Serial.println("senddeflat");
    fileManager->sendHeader("Content-Encoding", "deflate");
    fileManager->send_P(200, "text/html", ESPFMfGKWpindexpageDeflate, sizeof(ESPFMfGKWpindexpageDeflate));
    return;
  }
  if (uri == "/fm.css")
  {
    //Serial.println("senddeflat");
    fileManager->sendHeader("Content-Encoding", "deflate");
    fileManager->send_P(200, "text/css", ESPFMfGKWpcssDeflate, sizeof(ESPFMfGKWpcssDeflate));
    return;
  }
  if (uri == "/fm.js")
  {
    //Serial.println("senddeflat");
    fileManager->sendHeader("Content-Encoding", "deflate");
    fileManager->send_P(200, "text/javascript", ESPFMfGKWpjavascriptDeflate, sizeof(ESPFMfGKWpjavascriptDeflate));
    return;
  }
#endif

#ifndef fileManagerServerStaticsInternally
  if (uri == "/")
  {
    uri = "/fm.html";
  }
  if (isFileManagerInternalFile(uri))
  {
    // limits
    if (FileSystemIndexForWebPages < 0)
    {
      FileSystemIndexForWebPages = 0;
    }
    else if (FileSystemIndexForWebPages > maxfilesystem - 1)
    {
      FileSystemIndexForWebPages = maxfilesystem - 1;
    }
    servefile(uri);
    return;
  }
#endif

  if (checkURLs)
  {
    int res = checkURLs(uri);
    /** /
    Serial.print("CheckURLs res: ");
    Serial.println(res);
    /**/
    switch (res)
    {
    case 1:
      servefile(uri);
      return;
    case 2:
      fileManager->send(res, F("text/html"), uri);
      return;
    default:
      fileManager->send(404, F("text/plain"), F("URI not found."));
      return;
    }
  }
  else
  {
    servefile(uri);
    return;
  }
}

//*****************************************************************************************************
String ESPFMfGK::dispIntDotted(size_t i)
{
  String res = "";
  while (i != 0)
  {
    int r = i % 1000;
    res = String(i % 1000) + res;
    i /= 1000;
    if ((r < 100) && (i > 0))
    {
      res = "0" + res;
      if (r < 10)
      {
        res = "0" + res;
      }
    }
    if (i != 0)
    {
      res = "." + res;
    }
  }
  return res;
}

//*****************************************************************************************************
String ESPFMfGK::dispFileString(uint64_t fs, bool printorg)
{
  if (fs < 0)
  {
    return "-0";
  }

  if (fs == 0)
  {
    return "0 B";
  }

  if (fs < 1000)
  {
    return String(fs) + " B";
  }

  String units[] = {"B", "kB", "MB", "GB", "TB"}; // Yes, small k, large everything else..., SI-Präfix
  int digitGroups = (int)(log10(fs) / log10(1024));
  if (printorg)
  {
    return String(fs / pow(1024, digitGroups)) + " " + units[digitGroups] + " <small>(" + dispIntDotted(fs) + " B)</small>";
  }
  else
  {
    return String(fs / pow(1024, digitGroups)) + " " + units[digitGroups];
  }
}

//*****************************************************************************************************
void ESPFMfGK::fileManagerIndexpage(void)
{
  fileManager->send(200, F("text/html"), FPSTR(ESPFMfGKWpindexpage));
}

//*****************************************************************************************************
void ESPFMfGK::fileManagerJS(void)
{
  fileManager->send(200, F("text/javascript"), FPSTR(ESPFMfGKWpjavascript));
}

//*****************************************************************************************************
void ESPFMfGK::fileManagerCSS(void)
{
  fileManager->send(200, F("text/css"), FPSTR(ESPFMfGKWpcss));
}

//*****************************************************************************************************
String ESPFMfGK::CheckFileNameLengthLimit(String fn)
{
  // SPIFFS file name limit. Is there a way to get the max length from SPIFFS/LittleFS?
  //                                      SPIFFS_OBJ_NAME_LEN is spifLittleFS.... but not very clean.
  if (fn.length() > 32)
  {
    int len = fn.length();
    fn.remove(29);
    fn += String(len);
  }

  return fn;
}

//*****************************************************************************************************
String ESPFMfGK::colorline(int i)
{
  if (i % 2 == 0)
  {
    return "ccu";
  }
  else
  {
    return "ccg";
  }
}

//*****************************************************************************************************
bool ESPFMfGK::ShowInTreeView()
{
  if (fileManager->args() >= 1)
  {
    String tvs = fileManager->arg("t");
    return tvs == "true";
  }
  else
  {
    return false;
  }
}

//*****************************************************************************************************
String ESPFMfGK::CurrentPath()
{
  if (fileManager->args() >= 1)
  {
    return fileManager->arg("pn");
  }
  else
  {
    return "";
  }
}

//*****************************************************************************************************
int ESPFMfGK::getFileSystemIndex(bool uselastFileSystemIndex)
{
  if (fileManager->args() >= 1)
  {
    String fsis = fileManager->arg("fs");
    int fsi = fsis.toInt();
    if ((fsi >= 0) && (fsi <= sizeof(fsinfo) / sizeof(fsinfo[0])))
    {
      /*
        Serial.print("FS: ");
        Serial.print(fsinfo[fsi].fsname);
        Serial.println();
      */
      lastFileSystemIndex = fsi;
      return fsi;
    }
  }

  if ((uselastFileSystemIndex) && (lastFileSystemIndex != -1))
  {
    return lastFileSystemIndex;
  }

  return 0;
}

/*
  der file insert block
    %fn filename
    %fs filesize
    %cc ccu / ccg aus colorline()

  <div class="cc">
  <div class="gc">

    <div class="ccl %cc" onclick="downloadfile('%fn')">&nbsp;&nbsp;%fn</div>
    <div class="cct %cc">&nbsp;%fs&nbsp;</div>

    <div class="ccr %cc">&nbsp;
      <button title="Delete" onclick="deletefile('%fn')" class="b">D</button>
      <button title="Rename" onclick="renamefile('%fn')" class="b">R</button> &nbsp;&nbsp;
    </div>

  </div>
  </div>
*/

//*****************************************************************************************************
// flat view
void ESPFMfGK::recurseFolderList(String foldername, int maxtiefe, int tiefe)
{
  int fsi = getFileSystemIndex(false);

  // Schritt 1: die Ordner
  File root = fsinfo[fsi].filesystem->open(foldername);
  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      uint32_t flags = ~0;
      // Show this folder?
      if (checkFileFlags != NULL)
      {
        flags = checkFileFlags(*fsinfo[fsi].filesystem, String(file.path()) + "/", flagCheckIsPathname);
      }
      if (!(flags & ESPFMfGK::flagIsNotVisible))
      {
        if (!(String(file.path()).startsWith(svi)))
        {
          /** /
          Serial.print("Pfad: ");
          Serial.println(String(file.path()));
          Serial.print("Name: ");
          Serial.println(String(file.name()));
          /**/
          if (checkFileFlags != NULL)
          {
            flags = checkFileFlags(*fsinfo[fsi].filesystem, String(file.path()) + "/", flagCheckIsPathname);
          }
          if (!(flags & ESPFMfGK::flagIsNotVisible))
          {
            fileManager->sendContent(String(tiefe) + ":" + DeUmlautFilename(String(file.path())));
            fileManager->sendContent(itemtrenner); // 0
          }
        }
        if (tiefe < maxtiefe)
        {
          recurseFolderList(file.path(), maxtiefe, tiefe + 1);
        }
      }
    }
    file = root.openNextFile();
  }
}

//*****************************************************************************************************
String ESPFMfGK::Folder1LevelUp(String foldername)
{
  /** /
Serial.println(foldername);
  /**/
  int i = foldername.length();
  while ((i > 0) && (foldername.charAt(i) != '/'))
  {
    i--;
  }
  if (i > 0)
  {
    /** /
    Serial.println(foldername.substring(0, i));
    /**/
    return foldername.substring(0, i);
  }
  else
  {
    return "/";
  }
}

//*****************************************************************************************************
void ESPFMfGK::recurseFolder(String foldername, bool flatview, int maxtiefe, bool iststart, int &linecounter)
{
  /*
  Flatview steuert, ob eine Ordnerliste für den Ordner vorangestellt wird.
    bei der flatview werden alle dateien in einer langen list mit vorangestelltem
      ordnernamen ausgegeben
    bei !flatview wird nur der ordner ausgegeben, vorangestellt ist die
      ordnerliste incl. der ebene des ordnernamens.
  */
  int fsi = getFileSystemIndex(false);
  /** /
  Serial.print("fsi: ");
  Serial.print(fsi);
  Serial.println();
  /**/

  if ((!flatview) && (iststart))
  {
    if (foldername != "/")
    {
      String cache = "-1:..:" + Folder1LevelUp(foldername);
      cache += itemtrenner;
      cache += "-2:" + foldername;
      cache += itemtrenner;
      fileManager->sendContent(cache);

      /** /
      fileManager->sendContent("-1:..:" + Folder1LevelUp(foldername));
      fileManager->sendContent(itemtrenner); // 0
      fileManager->sendContent("-2:" + foldername);
      fileManager->sendContent(itemtrenner); // 1
      /**/
    }
    recurseFolderList(foldername, -1, 0);
  }

  // Trenner. Beim Start senden
  if (iststart)
  {
    fileManager->sendContent(beginoffiles);
  }

  //  Schritt 2: die Dateien in dem Ordner oder alles
  File root = fsinfo[fsi].filesystem->open(foldername);
  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      uint32_t flags = ~0;
      if (checkFileFlags != NULL)
      {
        flags = checkFileFlags(*fsinfo[fsi].filesystem, String(file.path()) + "/", flagCheckIsPathname);
      }
      if (!(flags & ESPFMfGK::flagIsNotVisible)) {
        if (flatview)
        {
          recurseFolder(file.path(), flatview, maxtiefe, false, linecounter);
        }
      }
    }
    else
    {
      // Serial.println(file.name());

      /* this is a bad solution, because what if the string changes?
         I couldn't find the source of this string in expressif/esp32/arduino sources.
      */

      if (!((fsinfo[fsi].filesystem == &SD) &&
            (String(file.path()).startsWith(svi))))
      {
        uint32_t flags = ~0;
        if (checkFileFlags != NULL)
        {
          flags = checkFileFlags(*fsinfo[fsi].filesystem, file.name(), flagCheckIsFilename);
        }
        if (!gzipperexists)
        {
          flags &= (~ESPFMfGK::flagCanGZip);
        }

        if (!(flags & ESPFMfGK::flagIsNotVisible))
        {
          String cache = String(file.path());
          cache += itemtrenner;
          cache += DeUmlautFilename(String(file.path()));
          cache += itemtrenner;
          cache += dispFileString(file.size(), false);
          cache += itemtrenner;
          cache += colorline(linecounter);
          cache += itemtrenner;
          cache += String(flags);
          cache += itemtrenner;
          fileManager->sendContent(cache);

          /** /
          fileManager->sendContent(String(file.path()));                   // .path() ist fqfn, .name() nur fn?
          fileManager->sendContent(itemtrenner);                           // 0
          fileManager->sendContent(DeUmlautFilename(String(file.path()))); // Display Name
          fileManager->sendContent(itemtrenner);                           // 1
          fileManager->sendContent(dispFileString(file.size(), false));
          fileManager->sendContent(itemtrenner); // 2
          fileManager->sendContent(colorline(linecounter));
          fileManager->sendContent(itemtrenner); // 3
          fileManager->sendContent(String(flags));
          fileManager->sendContent(itemtrenner); // 4
          /**/
          linecounter++;
        }
      }
    }
    file = root.openNextFile();
  }
}

//*****************************************************************************************************
void ESPFMfGK::fileManagerFileListInsert(void)
{
  // get the file system. all safe.
  int fsi = getFileSystemIndex();
  bool sit = ShowInTreeView();
  String path = CurrentPath();
  int maxtiefe = 0;
  for (int i = 0; i < path.length(); i++)
  {
    if (path.charAt(i) == '/')
    {
      maxtiefe++;
    }
  }

  // Flat view: immer im Root beginnen
  if ((!sit) || (path == ""))
  {
    path = "/";
  }

  /** /
  Serial.print("fsi: ");
  Serial.print(fsi);
  Serial.print(" sit: ");
  Serial.print(sit);
  Serial.print(" maxtiefe: ");
  Serial.print(maxtiefe);
  Serial.print(" path: ");
  Serial.print(path);
  Serial.println();
  /**/

  fileManager->setContentLength(CONTENT_LENGTH_UNKNOWN);
  fileManager->send(200, F("text/html"), String());

  gzipperexists = ((fsinfo[fsi].filesystem->exists("/gzipper.js.gz")) ||
                   (fsinfo[fsi].filesystem->exists("/gzipper.js")));

  int linecounter = 0;
  recurseFolder(path, !sit, maxtiefe, true, linecounter);

  String cache = antworttrenner + "<span title=\"";

  for (uint8_t i = 0; i < maxfilesystem; i++)
  {
    cache += "FS " + String(i) + ": " + fsinfo[i].fsname + "\n";
  }
  cache += "\">&nbsp; Size: " +
           dispFileString(totalBytes(fsinfo[fsi].filesystem), true) +
           ", used: " +
           dispFileString(usedBytes(fsinfo[fsi].filesystem), true) +
           "</span>";

  cache += antworttrenner;
  cache += "<select id=\"memory\" name=\"memory\" onchange=\"fsselectonchange();\">";
  for (int i = 0; i < maxfilesystem; i++)
  {
    if (i == fsi)
    {
      cache += "<option selected";
    }
    else
    {
      cache += "<option";
    }
    cache += ">" + fsinfo[i].fsname + "</option>";
  }
  cache += "</select>";

  cache += "<input type=\"checkbox\" id=\"treeview\" name=\"treeview\" onchange=\"fsselectonchange();\"";
  if (ShowInTreeView())
  {
    cache += " checked ";
  }
  cache += "/><label for=\"treeview\">Folders</label>";

  fileManager->sendContent(cache);
  // The End.
  fileManager->sendContent("");
}

//*****************************************************************************************************
String ESPFMfGK::escapeHTMLcontent(String html)
{
  // html.replace("<","&lt;");
  // html.replace(">","&gt;");
  html.replace("&", "&amp;");

  return html;
}

//*****************************************************************************************************
void ESPFMfGK::fileManagerBootinfo(void)
{
  // hier kann man die globalen Stati initialisieren, weil man weiß, dass die Webseite gerade frisch geladen wird.
  lastFileSystemIndex = -1;

  String fsinfos = "";
  for (uint8_t i = 0; i < maxfilesystem; i++)
  {
    fsinfos += String(i) + ": " + fsinfo[i].fsname + " ";
  }

  String cache =             //
      BackgroundColor +      //
      extrabootinfotrenner + //
      ExtraHTMLfoot +        //
      extrabootinfotrenner + //
      WebPageTitle +         //
      extrabootinfotrenner +
      fsinfos +
      extrabootinfotrenner +
      HtmlIncludes;

  fileManager->send(200, F("text/html"), cache);
}

//*****************************************************************************************************
// set the correct storage system, verifies the access and giives back the file name
String ESPFMfGK::getFileNameFromParam(uint32_t flag)
{
  /* the url looks like
       job?fs=xx&fn=filename&job=jobtoken
     plus extra params for some jobs.
  */
  /** /
  Serial.println("Params");
  for (int i = 0; i < fileManager->args(); i++)
  {
    Serial.print(fileManager->argName(i));
    Serial.print("=");
    Serial.print(fileManager->arg(i));
    Serial.println();
  }
  /**/

  // No flags, do nothing
  if (checkFileFlags == NULL)
  {
    /** /
    Serial.println("CheckFileFlags==NULL");
    /**/
    return "";
  }

  if (fileManager->args() < 3)
  {
    /** /
    Serial.println("Args < 3");
    /**/
    return "";
  }

  String fn = fileManager->arg("fn");

  if (fn == "")
  {
    /** /
    Serial.println("arg(fn) is empty");
    /**/
    return "";
  }

  int fsi = getFileSystemIndex();

  // Sonderregel, wenn eine neue Datei erstellt werden soll
  if ((flag & flagCanCreateNew) || (flag & flagAllowInZip))
  {
    return fn;
  }
  else
  {
    if (fsinfo[fsi].filesystem->exists(fn))
    { // file exists!
      if (checkFileFlags(*fsinfo[fsi].filesystem, fn, flagIsValidAction | flag) & flag == 0)
      {
        /** /
            Serial.println("checkFileFlags fail.");
        /**/
        return "";
      }

      // Yeah.
      return fn;
    }
  }

  /** /
Serial.println("Return nothing");
  /**/
  return "";
}

//*****************************************************************************************************
String ESPFMfGK::pathname(String fn)
{
  // find last "/"
  int i = fn.lastIndexOf("/");

  if (i > -1)
  {
    // Serial.println(fn.substring(0, i));
    return fn.substring(0, i);
  }
  else
  {
    return "/";
  }
}

//*****************************************************************************************************
int ESPFMfGK::getFSidxfromFilename(String fn)
{
  int i = fn.indexOf(":");
  if (i > -1)
  {
    fn = fn.substring(0, i - 1);
    int fnidx = fn.toInt();
    // Limits
    if ((fnidx < 0) || (fnidx >= maxfilesystem))
    {
      return -1;
    }
    else
    {
      return fnidx;
    }
  }
  else
  {
    return -1;
  }
}

//*****************************************************************************************************
String ESPFMfGK::getCleanFilename(String fn)
{
  int i = fn.indexOf(":");
  if (i > -1)
  {
    return fn.substring(i + 1, fn.length() - 2);
  }
  else
  {
    return fn;
  }
}

//*****************************************************************************************************
bool ESPFMfGK::CopyMoveFile(String oldfilename, String newfilename, bool move)
{
  // Zusammensuchen der FilesystemIndizes
  int fsiofn = getFSidxfromFilename(oldfilename);
  int fsinfn = getFSidxfromFilename(newfilename);

  if (fsiofn == -1)
  {
    fsiofn = getFileSystemIndex();
  }
  if (fsinfn == -1)
  {
    fsinfn = fsiofn;
  }

  // Aufräumen der Dateinamen
  oldfilename = getCleanFilename(oldfilename);
  newfilename = getCleanFilename(newfilename);

  // Neuen Ordner bauen, vorsichtshalber. Stückweise.
  int i = 1;
  String pn = pathname(newfilename);
  while (i < pn.length())
  {
    if (pn.charAt(i) == '/')
    {
      fsinfo[fsinfn].filesystem->mkdir(pn.substring(0, i));
    }
    i++;
  }
  fsinfo[fsinfn].filesystem->mkdir(pn);

  File oldfile = fsinfo[fsiofn].filesystem->open(oldfilename, FILE_READ);
  File newfile = fsinfo[fsinfn].filesystem->open(newfilename, FILE_WRITE);

  if ((oldfile) && (newfile))
  {
    const int bufsize = 4 * 1024;
    uint8_t *buffer;
    buffer = new uint8_t[4 * 1024];

    int bytesread = 0;
    int byteswritten = 0;
    while (oldfile.available())
    {
      size_t r = oldfile.read(buffer, bufsize);
      bytesread += r;
      byteswritten += newfile.write(buffer, r);
    }

    delete[] buffer;

    oldfile.close();
    newfile.close();

    // remove only, if new file is fully written.
    if ((move) && (bytesread == byteswritten))
    {
      fsinfo[fsiofn].filesystem->remove(oldfilename);
    }

    return true;
  }
  else
  {
    if (oldfile)
    {
      Serial.println(F("CMF: newfile fail."));
    }
    else
    {
      Serial.println(F("CMF: oldfile fail."));
    }
    return false;
  }
}

//*****************************************************************************************************
// Handles the command processing send from the web page
// sets the file system, checks the allowance flags and file existance
void ESPFMfGK::fileManagerJobber(void)
{
  if (fileManager->args() >= 3)
  { // https://www.youtube.com/watch?v=KSxTxynXiBs
    String jobname = fileManager->arg("job");
    if (jobname == "del")
    {
      String fn = getFileNameFromParam(flagCanDelete);
      /** /
      Serial.print("Delete: ");
      Serial.print(fn);
      Serial.println();
      /**/
      if (fn == "")
      {
        Illegal404();
        return;
      }
      fsinfo[getFileSystemIndex()].filesystem->remove(fn);
      // dummy answer
      fileManager->send(200, "text/plain", "");
      // Raus.
      return; //<<==========================
    }
    else if (jobname == "ren")
    {
      String fn = getFileNameFromParam(flagCanRename);
      String newfn = fileManager->arg("new");
      /** /
      Serial.print("Rename: ");
      Serial.print(fn);
      Serial.print(" new: ");
      Serial.print(newfn);
      Serial.println();
      /**/
      if (fn == "")
      {
        Illegal404();
        return;
      }
      if (!newfn.startsWith("/"))
      {
        newfn = "/" + newfn;
      }
      int fsi = getFileSystemIndex();

      if ((checkFileFlags(*fsinfo[fsi].filesystem, newfn, flagCanRename | flagIsValidTargetFilename) & flagCanRename) == 0)
      {
        Serial.println("Ren: No access.");
        Illegal404();
        return;
      }

      if (pathname(fn) == pathname(newfn))
      {
        if (!fsinfo[fsi].filesystem->rename(fn, newfn))
        {
          Serial.println(F("Rename failed (1)."));
        }
      }
      else
      {
        if (!CopyMoveFile(fn, newfn, true))
        {
          Serial.println(F("Rename failed (2)."));
        }
      }
      // dummy answer
      fileManager->send(200, "text/plain", "");
      // Raus.
      return; //<<==========================
    }
    else if (jobname == "edit")
    {
      String fn = getFileNameFromParam(flagCanEdit);
      /** /
      Serial.print("Edit: ");
      Serial.print(fn);
      Serial.println();
      /**/
      if (fn == "")
      {
        Illegal404();
        return;
      }
      fileManagerFileEditorInsert(fn);
      return; //<<==========================
    }
    else if (jobname == "dwnldll") // downloadall
    {
      String fn = getFileNameFromParam(flagAllowInZip | flagCanDownload);
      /** /
      Serial.print("Download: ");
      Serial.print(fn);
      Serial.println();
      /**/
      if (fn == "")
      {
        Illegal404();
        return;
      }
      {
#ifdef ZipDownloadAll
        int mode = fileManager->arg("mode").toInt();
        if (mode > 0)
        {
          ESPFMfGKGa *z = new ESPFMfGKGa();
          z->fileManager = fileManager;
          z->checkFileFlags = checkFileFlags;
          /** /
          Serial.print("FSI: ");
          Serial.print(getFileSystemIndex());
          Serial.print(" folder: ");
          Serial.print(fileManager->arg("folder"));
          Serial.print(" mode: ");
          Serial.print(mode);
          Serial.println();
          /**/
          z->getAllFilesInOneZIP(fsinfo[getFileSystemIndex()].filesystem, fileManager->arg("folder"), mode);
          delete z;
        }
        else
        {
          Illegal404();
        }
#else
        Illegal404();
#endif
      }
      return; //<<==========================
    }
    else if ((jobname == "download") || (jobname == "preview"))
    {
      String fn = getFileNameFromParam(flagCanDownload);
      /** /
      Serial.print(F("Download: "));
      Serial.print(fn);
      Serial.println();
      /**/
      if (fn == "")
      {
        Illegal404();
        return;
      }
      if (jobname == "download")
      {
        fileManagerDownload(fn);
      }
      else
      {
        servefile(fn);
      }
      return; //<<==========================
    }
    else if (jobname == "createnew")
    {
      String fn = getFileNameFromParam(flagCanCreateNew);
      // benötigt einen Filenamen-Fragment als Parameter, <nummer>.txt wird hier angefügt
      /** /
      Serial.print(F("CreateNew: "));
      Serial.print(fn);
      Serial.println();
      /**/
      if (!fn.startsWith("/"))
      {
        fn = "/" + fn;
      }
      if (fn == "")
      {
        Illegal404();
        return;
      }
      int fsi = getFileSystemIndex();

      int index = 0;
      while (fsinfo[fsi].filesystem->exists(fn + String(index) + ".txt"))
      {
        index++;
      }
      File file = fsinfo[fsi].filesystem->open(fn + String(index) + ".txt", FILE_WRITE);
      file.close();

      fileManager->send(200, "text/plain", "");
      return; //<<==========================
    }
  }

  // in case all fail, ends here
  Illegal404();
}

//*****************************************************************************************************
void ESPFMfGK::HtmlIncludesInterface(void)
{
  if (HtmlIncludesCallback)
  {
    if (!HtmlIncludesCallback(fileManager))
    {
      Illegal404();
    }
  }
  else
  {
    Illegal404();
  }
}

//*****************************************************************************************************
void ESPFMfGK::Illegal404()
{
  Serial.println(F("FileManager: send 404."));
  // in case all fail, ends here
  fileManager->send(404, F("text/plain"), F("Illegal."));
}

// in place editor
//*****************************************************************************************************
void ESPFMfGK::fileManagerFileEditorInsert(String &filename)
{
  // Serial.println("Edit");
  fileManager->setContentLength(CONTENT_LENGTH_UNKNOWN);
  fileManager->send(200, F("text/html"), String());

  fileManager->sendContent(ESPFMfGKWpFormIntro1);
  fileManager->sendContent(textareaCharset);
  fileManager->sendContent(ESPFMfGKWpFormIntro2);

  if (fsinfo[getFileSystemIndex()].filesystem->exists(filename))
  {
    File f = fsinfo[getFileSystemIndex()].filesystem->open(filename, "r");
    if (f)
    {
      const int chuncksize = 2 * 1024;
      String cache = "";
      do
      {
        cache += f.readStringUntil('\n') + '\n';
        if (cache.length() >= chuncksize)
        {
          cache = escapeHTMLcontent(cache);
          fileManager->sendContent(cache);
          cache = "";
        }
      } while (f.available());
      f.close();
      if (cache != "")
      {
        fileManager->sendContent(cache);
      }
    }
  }
  else
  {
    /** /
    Serial.println(filename);
    Serial.println(F("File not found"));
    /**/
  }

  fileManager->sendContent(ESPFMfGKWpFormExtro1);
  fileManager->sendContent("");
}

//*****************************************************************************************************
// Download: open url in binary mode.
void ESPFMfGK::fileManagerDownload(String &filename)
{
  // filesystem set by caller
  File f = fsinfo[getFileSystemIndex()].filesystem->open(filename, "r");
  if (f)
  {
    // ohne führend slash
    if (filename.startsWith("/"))
    {
      filename.remove(0, 1);
    }
    fileManager->sendHeader(F("Content-Type"), F("text/text"));
    fileManager->sendHeader(F("Connection"), F("close"));
    fileManager->sendHeader(F("Content-Disposition"), "attachment; filename=" + filename);
    fileManager->sendHeader(F("Content-Transfer-Encoding"), F("binary"));
    if (fileManager->streamFile(f, "application/octet-stream") != f.size())
    {
      Serial.println(F("Sent less data than expected!"));
    }
    f.close();
    return;
  }
}

// Drag and Drop
//   https://developer.mozilla.org/en-US/docs/Web/API/HTML_Drag_and_Drop_API/File_drag_and_drop
//   https://www.ab-heute-programmieren.de/drag-and-drop-upload-mit-html5/#Schritt_3_Eine_Datei_hochladen
//*****************************************************************************************************
void ESPFMfGK::fileManagerReceiverOK(void)
{
  // Serial.println("fileManagerReceiverOK");
  fileManager->send(200);
  delay(1);
}

//*****************************************************************************************************
void ESPFMfGK::fileManagerReceiver(void)
{
  HTTPUpload &upload = fileManager->upload();
  Serial.println("Server upload Status: " + String(upload.status));

  if (upload.status == UPLOAD_FILE_START)
  {
    /** /
    Serial.println("fileManagerReceiver: Start");

    Serial.println("Params");
    for (int i = 0; i < fileManager->args(); i++)
    {
      Serial.print(fileManager->argName(i));
      Serial.print("=");
      Serial.print(fileManager->arg(i));
      Serial.println();
    }
    /**/
    // in case we have a dangling open file....
    fsUploadFile.close();

    // No flags, do nothing
    if (checkFileFlags == NULL)
    {
      return;
    }

    if (fileManager->args() < 2)
    {
      return;
    }

    String fn = fileManager->arg("fn");
    if (fn == "")
    {
      return;
    }
    if (!fn.startsWith("/"))
    {
      fn = "/" + fn;
    }

    int fsi = getFileSystemIndex();

    // cut length
    fn = CheckFileNameLengthLimit(fn);

    // https://github.com/holgerlembke/ESPFMfGK/issues/13
    if ((checkFileFlags(*fsinfo[fsi].filesystem, fn, flagCanUpload) & flagCanUpload) == 0)
    {
      return;
    }

    if ((checkFileFlags(*fsinfo[fsi].filesystem, fn, flagIsValidTargetFilename) & flagIsValidTargetFilename) != 0)
    {
      return;
    }

    fsUploadFile = fsinfo[fsi].filesystem->open(fn, "w");
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    /** /
    Serial.print("handleFileUpload Data: ");
    Serial.println(upload.currentSize);
    /**/
    if (fsUploadFile)
    {
      /** /
      Serial.println("fileManagerReceiver: Write");
      /**/
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    /** /
    Serial.println("fileManagerReceiver: End");
    /**/
    if (fsUploadFile)
    {
      fsUploadFile.close();
      // fsUploadFile = NULL;
    }
    Serial.print("handleFileUpload Size: ");
    Serial.println(upload.totalSize);
  }
}

//*****************************************************************************************************
String ESPFMfGK::DeUmlautFilename(String fn)
{ // cp437/cp850 to ...
  String nfn = "";
  for (int i = 0; i < fn.length(); i++)
  {
    switch (fn[i])
    {
    case 0x84:
      nfn += "\u00e4";
      break;
    case 0x94:
      nfn += "\u00f6";
      break;
    case 0x81:
      nfn += "\u00fc";
      break;
    case 0x8E:
      nfn += "\u00c4";
      break;
    case 0x99:
      nfn += "\u00d6";
      break;
    case 0x9A:
      nfn += "\u00dc";
      break;
    case 0xE1:
      nfn += "\u00df";
      break;
      // €	\u20ac

    default:
      nfn += fn[i];
      break;
    }
  }

  return nfn;
}

//*****************************************************************************************************
// total/used are not exposed in FS::FS. Who knows why.
uint64_t ESPFMfGK::totalBytes(fs::FS *fs)
{
#ifdef SOC_SDMMC_HOST_SUPPORTED
  if (fs == &SD_MMC)
  {
    return SD_MMC.totalBytes();
  }
#endif  
  else if (fs == &SD)
  {
    return SD.totalBytes();
  }
  else if (fs == &LittleFS)
  {
    return LittleFS.totalBytes();
  }
  else if (fs == &FFat)
  {
    return FFat.totalBytes();
  }
  else
  {
    return -1;
  }
}

//*****************************************************************************************************
uint64_t ESPFMfGK::usedBytes(fs::FS *fs)
{
#ifdef SOC_SDMMC_HOST_SUPPORTED
  if (fs == &SD_MMC)
  {
    return SD_MMC.usedBytes();
  }
#endif  
  else if (fs == &SD)
  {
    return SD.usedBytes();
  }
  else if (fs == &LittleFS)
  {
    return LittleFS.usedBytes();
  }
  else if (fs == &FFat)
  {
    return FFat.usedBytes();
  }
  else
  {
    return -1;
  }
}
