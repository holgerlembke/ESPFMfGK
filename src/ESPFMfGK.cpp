#include <Arduino.h>
#include <inttypes.h>
#include <ESPFMfGK.h>
#include <ESPFMfGKWp.h>   // web page/javascript/css
#include <ESPFMfGKWpF2.h> // some form fragments

#include <crc32.h>

#include <WebServer.h>
#include <FS.h>
#include <SD.h>
#include <LittleFS.h>
#include <SD_MMC.h>
#include <FFat.h>
#include <detail/RequestHandlersImpl.h>

String getContentType(const String &path)
{
  return StaticRequestHandler::getContentType(path);
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
    return false;
  }

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
    Serial.print("CheckURLs res: ");
    Serial.println(res);
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
String ESPFMfGK::dispFileString(size_t fs, bool printorg)
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
      if (!(String(file.path()).startsWith(svi)))
      {
        /** /
        Serial.print("Pfad: ");
        Serial.println(String(file.path()));
        Serial.print("Name: ");
        Serial.println(String(file.name()));
        /**/
        uint32_t flags = ~0;
        if (checkFileFlags != NULL)
        {
          flags = checkFileFlags(*fsinfo[fsi].filesystem, String(file.path()) + "/", 0);
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
    file = root.openNextFile();
  }
}

//*****************************************************************************************************
String ESPFMfGK::Folder1LevelUp(String foldername)
{
  Serial.println(foldername);
  int i = foldername.length();
  while ((i > 0) && (foldername.charAt(i) != '/'))
  {
    i--;
  }
  if (i > 0)
  {
    Serial.println(foldername.substring(0, i));
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
  /**/
  Serial.print("fsi: ");
  Serial.print(fsi);
  Serial.println();
  /**/

  if ((!flatview) && (iststart))
  {
    if (foldername != "/")
    {
      fileManager->sendContent("-1:..:" + Folder1LevelUp(foldername));
      fileManager->sendContent(itemtrenner); // 0
      fileManager->sendContent("-2:" + foldername);
      fileManager->sendContent(itemtrenner); // 1
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
      if (flatview)
      {
        recurseFolder(file.path(), flatview, maxtiefe, false, linecounter);
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
          flags = checkFileFlags(*fsinfo[fsi].filesystem, file.name(), 0);
        }
        if (!gzipperexists)
        {
          flags &= (~ESPFMfGK::flagCanGZip);
        }

        if (!(flags & ESPFMfGK::flagIsNotVisible))
        {
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
  fileManager->sendContent(antworttrenner);

  String sinfo = "<span title=\"";

  for (uint8_t i = 0; i < maxfilesystem; i++)
  {
    sinfo += "FS " + String(i) + ": " + fsinfo[i].fsname + "\n";
  }
  sinfo += "\">&nbsp; Size: " +
           dispFileString(totalBytes(fsinfo[fsi].filesystem), true) +
           ", used: " +
           dispFileString(usedBytes(fsinfo[fsi].filesystem), true) +
           "</span>";
  /*
    fileManager->sendContent(F(" FS blocksize: "));
    fileManager->sendContent(String(info.blockSize));
    fileManager->sendContent(F(", pageSize: "));
    fileManager->sendContent(String(info.pageSize));
  */

  fileManager->sendContent(sinfo);

  fileManager->sendContent(antworttrenner);

  fileManager->sendContent(F("<select id=\"memory\" name=\"memory\" onchange=\"fsselectonchange();\">"));
  for (int i = 0; i < maxfilesystem; i++)
  {
    if (i == fsi)
    {
      fileManager->sendContent(F("<option selected"));
    }
    else
    {
      fileManager->sendContent(F("<option"));
    }
    fileManager->sendContent(">");
    fileManager->sendContent(fsinfo[i].fsname);
    fileManager->sendContent(F("</option>"));
  }
  fileManager->sendContent(F("</select>"));

  fileManager->sendContent(F("<input type=\"checkbox\" id=\"treeview\" name=\"treeview\" "));
  fileManager->sendContent(F("onchange=\"fsselectonchange();\""));
  if (ShowInTreeView())
  {
    fileManager->sendContent(F(" checked "));
  }
  fileManager->sendContent(F("/>"));
  fileManager->sendContent(F("<label for=\"treeview\">Folders</label>"));

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

  fileManager->setContentLength(CONTENT_LENGTH_UNKNOWN);
  fileManager->send(200, F("text/html"), String());

  fileManager->sendContent(BackgroundColor);
  fileManager->sendContent(extrabootinfotrenner);

  fileManager->sendContent(ExtraHTMLfoot);
  fileManager->sendContent(extrabootinfotrenner);

  fileManager->sendContent(WebPageTitle);
  fileManager->sendContent(extrabootinfotrenner);

  // The End.
  fileManager->sendContent("");
}

//*****************************************************************************************************
// set the correct storage system, verifies the access and giives back the file name
String ESPFMfGK::getFileNameFromParam(uint32_t flag)
{
  /* the url looks like
       job?fs=xx&fn=filename&job=jobtoken
     plus extra params for some jobs.
  */
  Serial.println("Params");
  for (int i = 0; i < fileManager->args(); i++)
  {
    Serial.print(fileManager->argName(i));
    Serial.print("=");
    Serial.print(fileManager->arg(i));
    Serial.println();
  }

  // No flags, do nothing
  if (checkFileFlags == NULL)
  {
    return "";
  }

  if (fileManager->args() < 3)
  {
    return "";
  }

  String fn = fileManager->arg("fn");

  if (fn == "")
  {
    return "";
  }

  int fsi = getFileSystemIndex();

  if (fsinfo[fsi].filesystem->exists(fn))
  { // file exists!
    if (checkFileFlags(*fsinfo[fsi].filesystem, fn, flagIsValidAction | flag) & flag == 0)
    {
      return "";
    }

    // Yeah.
    return fn;
  }

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
      /**/
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
      /**/
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

      if (checkFileFlags(*fsinfo[fsi].filesystem, newfn, flagCanRename | flagIsValidTargetFilename) & flagCanRename == 0)
      {
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
      /**/
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
    else if ((jobname == "download") || (jobname == "preview"))
    {
      String fn = getFileNameFromParam(flagCanDownload);
      /**/
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
      /**/
      Serial.print(F("CreateNew: "));
      Serial.print(fn);
      Serial.println();
      /**/
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
      do
      {
        String l = f.readStringUntil('\n') + '\n';
        l = escapeHTMLcontent(l);
        fileManager->sendContent(l);
      } while (f.available());
      f.close();
    }
  }
  else
  {
    Serial.println(filename);
    Serial.println(F("File not found"));
  }

  fileManager->sendContent(ESPFMfGKWpFormExtro1);
  fileManager->sendContent(filename);
  fileManager->sendContent(ESPFMfGKWpFormExtro2);

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
    /**/
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

    if (checkFileFlags(*fsinfo[fsi].filesystem, fn, flagCanUpload | flagIsValidTargetFilename) & flagCanUpload == 0)
    {
      return;
    }

    fsUploadFile = fsinfo[fsi].filesystem->open(fn, "w");
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    Serial.print("handleFileUpload Data: ");
    Serial.println(upload.currentSize);
    if (fsUploadFile)
    {
      /**/
      Serial.println("fileManagerReceiver: Write");
      /**/
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    /**/
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

struct __attribute__((__packed__)) zipFileHeader
{
  uint32_t signature; // 0x04034b50;
  uint16_t versionneeded;
  uint16_t bitflags;
  uint16_t comp_method;
  uint16_t lastModFileTime;
  uint16_t lastModFileDate;
  uint32_t crc_32;
  uint32_t comp_size;
  uint32_t uncompr_size;
  uint16_t fname_len;
  uint16_t extra_field_len;
};

struct __attribute__((__packed__)) zipDataDescriptor
{
  uint32_t signature; // 0x08074b50
  uint32_t crc32;
  uint32_t comp_size;
  uint32_t uncompr_size;
};

struct __attribute__((__packed__)) zipEndOfDirectory
{
  uint32_t signature; // 0x06054b50;
  uint16_t nrofdisks;
  uint16_t diskwherecentraldirectorystarts;
  uint16_t nrofcentraldirectoriesonthisdisk;
  uint16_t totalnrofcentraldirectories;
  uint32_t sizeofcentraldirectory;
  uint32_t ofsetofcentraldirectoryrelativetostartofarchiv;
  uint16_t commentlength;
};

struct __attribute__((__packed__)) zipCentralDirectoryFileHeader
{
  uint32_t signature; // 0x02014b50
  uint16_t versionmadeby;
  uint16_t versionneededtoextract;
  uint16_t flag;
  uint16_t compressionmethode;
  uint16_t lastModFileTime;
  uint16_t lastModFileDate;
  uint32_t crc_32;
  uint32_t comp_size;
  uint32_t uncompr_size;
  uint16_t fname_len;
  uint16_t extra_len;
  uint16_t comment_len;
  uint16_t diskstart;
  uint16_t internalfileattr;
  uint32_t externalfileattr;
  uint32_t relofsoflocalfileheader;
  // nun filename, extra field, comment
};

//*****************************************************************************************************
int ESPFMfGK::WriteChunk(const char *b, size_t l)
{
  //  Serial.print(" Chunk: " + String(l) + " ");

  const char *footer = "\r\n";
  char chunkSize[11];
  sprintf(chunkSize, "%zx\r\n", l);
  fileManager->client().write(chunkSize, strlen(chunkSize));
  fileManager->client().write(b, l);
  fileManager->client().write(footer, 2);

  return strlen(chunkSize) + l + 2;
}

//*****************************************************************************************************
/* https://en.wikipedia.org/wiki/Zip_(file_format)
   https://www.fileformat.info/tool/hexdump.htm
   https://hexed.it/?hl=de
   HxD https://mh-nexus.de/de/

   This code needs some memory:
     4 * <nr. of files> + copybuffersize

   Uses no compression, because, well, code size. Should be good for 4mb.
*/
void ESPFMfGK::getAllFilesInOneZIP(void)
{
  const byte copybuffersize = 100;

  fileManager->setContentLength(CONTENT_LENGTH_UNKNOWN);
  // fileManager->sendHeader(F("Content-Type"), F("text/text"));
  // fileManager->sendHeader(F("Transfer-Encoding"), F("chunked"));
  // fileManager->sendHeader(F("Connection"), F("close"));
  fileManager->sendHeader(F("Content-Disposition"), F("attachment; filename=alles.zip"));
  fileManager->sendHeader(F("Content-Transfer-Encoding"), F("binary"));
  fileManager->send(200, F("application/octet-stream"), "");

  // get the file system. all safe.
  int fsi = getFileSystemIndex();

  // Pass 0: count files
  int files = 0;
  {
    File file = fsinfo[fsi].filesystem->open("/");
    while (file)
    {
      String fn = file.name();
      /*
        if ( (_ViewSysFiles) || (allowAccessToThisFile(fn)) ) {
        files++;
        }
      */
      file = file.openNextFile();
    }
    // Serial.println("Files: "+String(files));
  }
  // Store the crcs
  uint32_t crc_32s[files];

  // Pass 1: local headers + file
  {
    zipFileHeader zip;
    zip.signature = 0x04034b50;
    zip.versionneeded = 0;
    zip.bitflags = 1 << 3;
    zip.comp_method = 0; // stored
    zip.lastModFileTime = 0x4fa5;
    zip.lastModFileDate = 0xe44e;
    zip.extra_field_len = 0;

    int i = 0;
    File file = fsinfo[fsi].filesystem->open("/");
    while (file)
    {
      String fn = file.name();

      // if ( (_ViewSysFiles) || (allowAccessToThisFile(fn)) ) {
      if (fn.indexOf("/") == 0)
      {
        fn.remove(0, 1); // "/" filenames beginning with "/" dont work for Windows....
      }

      zip.comp_size = 0;
      zip.uncompr_size = 0;
      zip.crc_32 = 0;
      zip.fname_len = fn.length();
      WriteChunk((char *)&zip, sizeof(zip));
      WriteChunk(fn.c_str(), zip.fname_len);

      //        Serial.print("Send: " + fn);
      // File f = dir.open("r",FILE_READ);
      int len = file.size();

      // send crc+len later...
      zipDataDescriptor datadiscr;
      datadiscr.signature = 0x08074b50;
      datadiscr.comp_size = len;
      datadiscr.uncompr_size = len;

      const char *footer = "\r\n";
      char chunkSize[11];
      sprintf(chunkSize, "%zx\r\n", len);
      fileManager->client().write(chunkSize, strlen(chunkSize));

      { // pff.
        CRC32 crc;
        byte b[copybuffersize];
        int lenr = len;
        while (lenr > 0)
        {
          byte r = (lenr > copybuffersize) ? copybuffersize : lenr;
          file.read(b, r);
          crc.update(b, r);
          fileManager->client().write(b, r);
          lenr -= r;
        }
        datadiscr.crc32 = crc.finalize();
        crc_32s[i] = datadiscr.crc32;
      }

      fileManager->client().write(footer, 2);

      WriteChunk((char *)&datadiscr, sizeof(datadiscr));

      // f.close();
      i++;
      /** /
              Serial.print(" ");
              Serial.print(l);
              Serial.println();
        /**/
      //}
      file = file.openNextFile();
    }
  }

  // Pass 2: Central directory Structur
  {
    zipEndOfDirectory eod;
    eod.signature = 0x06054b50;
    eod.nrofdisks = 0;
    eod.diskwherecentraldirectorystarts = 0;
    eod.nrofcentraldirectoriesonthisdisk = 0;
    eod.totalnrofcentraldirectories = 0;
    eod.sizeofcentraldirectory = 0;
    eod.ofsetofcentraldirectoryrelativetostartofarchiv = 0;
    eod.commentlength = 0;

    zipCentralDirectoryFileHeader CDFH;

    CDFH.signature = 0x02014b50;
    CDFH.versionmadeby = 0;
    CDFH.versionneededtoextract = 0;
    CDFH.flag = 0;
    CDFH.compressionmethode = 0; // Stored
    CDFH.lastModFileTime = 0x4fa5;
    CDFH.lastModFileDate = 0xe44e;
    CDFH.extra_len = 0;
    CDFH.comment_len = 0;
    CDFH.diskstart = 0;
    CDFH.internalfileattr = 0x01;
    CDFH.externalfileattr = 0x20;
    CDFH.relofsoflocalfileheader = 0;

    int i = 0;

    File file = fsinfo[fsi].filesystem->open("/");
    while (file)
    {
      String fn = file.name();

      // if ( (_ViewSysFiles) || (allowAccessToThisFile(fn)) ) {
      if (fn.indexOf("/") == 0)
      {
        fn.remove(0, 1); // "/" filenames beginning with "/" dont work for Windows....
      }
      //        Serial.print("CDFH: " + fn);
      // File f = dir.open("r",FILE_READ);
      int len = file.size();

      CDFH.comp_size = len;
      CDFH.uncompr_size = len;
      CDFH.fname_len = fn.length();
      CDFH.crc_32 = crc_32s[i];

      // f.close();

      WriteChunk((char *)&CDFH, sizeof(CDFH));
      WriteChunk(fn.c_str(), CDFH.fname_len);

      int ofs = sizeof(zipFileHeader) + len + CDFH.fname_len + sizeof(zipDataDescriptor);

      // next position
      CDFH.relofsoflocalfileheader += ofs;

      // book keeping
      eod.nrofcentraldirectoriesonthisdisk++;
      eod.totalnrofcentraldirectories++;
      eod.ofsetofcentraldirectoryrelativetostartofarchiv += ofs;
      eod.sizeofcentraldirectory += sizeof(CDFH) + CDFH.fname_len;

      i++;
      //}
      file = file.openNextFile();
    }

    //    Serial.print("EOD: ");
    WriteChunk((char *)&eod, sizeof(eod));
    //    Serial.println();
  }

  const char *endchunk = "0\r\n\r\n";
  fileManager->client().write(endchunk, 5);

  fileManager->sendContent("");
  delay(1);
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
  if (fs == &SD_MMC)
  {
    return SD_MMC.totalBytes();
  }
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
  if (fs == &SD_MMC)
  {
    return SD_MMC.usedBytes();
  }
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

//*****************************************************************************************************
/*

  // +--++--++--++--++--++--++--++--++--++--++--++--++--++--++--+
  // one arg, "za", zip all and download
  if ( (fileManager->args() == 1) && (fileManager->argName(0) == "za") ) {
    getAllFilesInOneZIP();
    // does it all
    return;
  }
  }
*/
