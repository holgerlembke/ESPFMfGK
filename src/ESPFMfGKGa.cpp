#include <Arduino.h>
#include <inttypes.h>
#include <ESPFMfGKGa.h>
#include <FS.h>

#include <crc32.h>

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
void ESPFMfGKGa::deletefoldert(folder_t *root /**/)
{
  while (root)
  {
    folder_t *temp = root;
    root = root->next;
    delete temp;
  }
}

//*****************************************************************************************************
void ESPFMfGKGa::displayfoldert(folder_t *root /**/)
{
  while (root)
  {
    Serial.println(root->foldername);
    root = root->next;
  }
}

//*****************************************************************************************************
ESPFMfGKGa::folder_t *ESPFMfGKGa::buildfilelistrecurser(fs::FS &fs, String pfad, ESPFMfGKGa::folder_t *localroot /**/)
{
  File rootfile = fs.open(pfad);
  if (!rootfile)
  {
    Serial.println("- failed to open directory");
    return NULL;
  }
  if (!rootfile.isDirectory())
  {
    Serial.println(" - not a directory");
    return NULL;
  }

  File file = rootfile.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      /** /
      Serial.print("DIR: ");
      Serial.println(file.name());
      /**/

      localroot->next = new folder_t;
      localroot = localroot->next;
      localroot->foldername = file.path();
      localroot->next = NULL;

      localroot = buildfilelistrecurser(fs, file.path(), localroot);
    }
    file = rootfile.openNextFile();
  }
  return localroot;
}

//*****************************************************************************************************
ESPFMfGKGa::folder_t *ESPFMfGKGa::buildfilelist(fs::FS &fs /**/, String rootfolder)
{
  /** /
  Serial.print("buildfilelist root: ");
  Serial.println(rootfolder);
  /**/

  folder_t *root = new folder_t;

  root->next = NULL;
  root->foldername = rootfolder;
  buildfilelistrecurser(fs, rootfolder, root);

  return root;
}

//*****************************************************************************************************
int ESPFMfGKGa::WriteChunk(const char *b, size_t l)
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
   unzip -t <filename>

   This code needs some memory:
     4 * <nr. of files> + copybuffersize

   Uses no compression, because, well, code size. Should be good for 4mb.
*/
void ESPFMfGKGa::getAllFilesInOneZIP(fs::FS *fs, String rootfolder)
{
  const int copybuffersize = 1000;

  fileManager->setContentLength(CONTENT_LENGTH_UNKNOWN);
  // fileManager->sendHeader(F("Content-Type"), F("text/text"));
  // fileManager->sendHeader(F("Transfer-Encoding"), F("chunked"));
  // fileManager->sendHeader(F("Connection"), F("close"));
  fileManager->sendHeader(F("Content-Disposition"), F("attachment; filename=alles.zip"));
  fileManager->sendHeader(F("Content-Transfer-Encoding"), F("binary"));
  fileManager->send(200, F("application/octet-stream"), "");

  // Upsi.
  if (rootfolder == "")
  {
    rootfolder = "/";
  }

  // Pass -1: get folder list
  folder_t *folderlist = buildfilelist(*fs, rootfolder);
  /** /
  displayfoldert(folderlist);
  /**/

  uint32_t flags;

  // Pass 0: count files
  int files = 0;
  folder_t *folderi = folderlist;
  while (folderi)
  {
    {
      File root = fs->open(folderi->foldername);
      File file = root.openNextFile();
      while (file)
      {
        if (!file.isDirectory())
        {
          String fn = file.name();
          if (checkFileFlags != NULL)
          {
            flags = checkFileFlags(*fs, fn, ESPFMfGK::flagIsValidAction | ESPFMfGK::flagAllowInZip);
          }
          if (flags & ESPFMfGK::flagAllowInZip)
          {
            files++;
          }
        }
        file = root.openNextFile();
      }
    }
    folderi = folderi->next;
  }
  /** /
  Serial.print("Files: ");
  Serial.print(files);
  Serial.println();
  /**/
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

    folderi = folderlist;
    int fileidx = 0;
    while (folderi)
    {
      File root = fs->open(folderi->foldername);
      File file = root.openNextFile();
      while (file)
      {
        if (!file.isDirectory())
        {
          String fn = file.name();

          if (checkFileFlags != NULL)
          {
            flags = checkFileFlags(*fs, fn, ESPFMfGK::flagIsValidAction | ESPFMfGK::flagAllowInZip);
          }
          if (flags & ESPFMfGK::flagAllowInZip)
          {
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

            size_t len = file.size();

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
              size_t r;
              do
              {
                r = file.read(b, copybuffersize);
                if (r > 0)
                {
                  crc.update(b, r);
                  fileManager->client().write(b, r);
                }
              } while (r == copybuffersize);
              datadiscr.crc32 = crc.finalize();
              crc_32s[fileidx] = datadiscr.crc32;
            }
            fileManager->client().write(footer, 2);

            WriteChunk((char *)&datadiscr, sizeof(datadiscr));
            fileidx++;
          }
        }
        file = root.openNextFile();
      }
      folderi = folderi->next;
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

    folderi = folderlist;
    int fileidx = 0;
    while (folderi)
    {
      File root = fs->open(folderi->foldername);
      File file = root.openNextFile();
      while (file)
      {
        if (!file.isDirectory())
        {
          String fn = file.name();

          if (checkFileFlags != NULL)
          {
            flags = checkFileFlags(*fs, fn, ESPFMfGK::flagIsValidAction | ESPFMfGK::flagAllowInZip);
          }
          if (flags & ESPFMfGK::flagAllowInZip)
          {
            if (fn.indexOf("/") == 0)
            {
              fn.remove(0, 1); // "/" filenames beginning with "/" dont work for Windows....
            }
            size_t len = file.size();

            CDFH.comp_size = len;
            CDFH.uncompr_size = len;
            CDFH.fname_len = fn.length();
            CDFH.crc_32 = crc_32s[fileidx];

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

            fileidx++;
          }
        }
        file = root.openNextFile();
      }
      folderi = folderi->next;
    }

    WriteChunk((char *)&eod, sizeof(eod));
  }

  const char *endchunk = "0\r\n\r\n";
  fileManager->client().write(endchunk, 5);

  fileManager->sendContent("");

  deletefoldert(folderlist);
}
