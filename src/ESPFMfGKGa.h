#ifndef ESPFMfGKGa_h
#define ESPFMfGKGa_h

#include <FS.h>
#include <WebServer.h>
#include <ESPFMfGK.h>

/*
  Part of ESP32 File Manager for Generation Klick aka ESPFMfGK
    https://github.com/holgerlembke/ESPFMfGK
    lembke@gmail.com

  V1.0
    + Redesign ZIP-Schnittstelle, Verlagerung in eigenen .h/.cpp wg. Sourcecodeumfangsverminderung 
    + Auslagerung aus ESPFMfGK.h
    + läuft

*/

// ausgelagerte getAllFilesInOneZIP
class ESPFMfGKGa
{
private:
    // Simple linked list, neue Elemente werden HINTEN eingehängt
    struct folder_t
    {
        folder_t *next;
        String foldername;
    };

    void deletefoldert(folder_t *root /**/);
    void displayfoldert(folder_t *root /**/);
    folder_t *buildfilelistrecurser(fs::FS &fs, String pfad, folder_t *localroot /**/);
    folder_t *buildfilelist(fs::FS &fs, String rootfolder /**/);

    int WriteChunk(const char *b, size_t l);

public:
    WebServer *fileManager = NULL;
    ESPxWebCallbackFlags_t checkFileFlags = NULL;

    void getAllFilesInOneZIP(fs::FS *fs, String rootfolder);
};

#endif
