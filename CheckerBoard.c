 #include <intrin.h>
 #include <vector>
 #include <algorithm>
+#include <QLibrary> // For loading shared libraries (.dll, .so, .dylib)
+#include <QDir>     // For path manipulation
+#include <QString>  // For path manipulation
+#include <QDebug>   // For warnings/logging

 #include "standardheader.h"
 #include "cb_interface.h"
 // ... (rest of includes) ...

 // --- Remove Global HINSTANCE handles ---
 // These will be managed by the C++ MainWindow or an EngineManager class
 // HINSTANCE hinstLib1 = NULL;
 // HINSTANCE hinstLib2 = NULL;
 // ------------------------------------

 // Global engine function pointers (still needed for C functions calling them)
 CB_GETMOVE getmove1 = NULL, getmove2 = NULL, getmove = NULL;
 CB_ISLEGAL islegal1 = NULL, islegal2 = NULL, islegal = NULL;
 CB_ENGINECOMMAND enginecommand1 = NULL, enginecommand2 = NULL;
 CB_GETSTRING enginename1 = NULL, enginename2 = NULL;
 CB_GETGAMETYPE CBgametype = NULL; // Assuming this stays global for now

 // ... (rest of globals) ...

 // --- Refactor load_engine ---
 /*
  * Load an engine shared library (.dll, .so, .dylib), and get pointers to the exported functions.
  * Return non-zero on error.
  * NOTE: This C function is a temporary step. Engine loading should ideally be
  * managed entirely within a C++ class (e.g., EngineManager or MainWindow)
  * which would store the QLibrary instances and function pointers.
  */
 int load_engine_qt(
     QLibrary **lib_ptr, // Pointer to a QLibrary pointer
     const char *dllname,
     CB_ENGINECOMMAND *cmdfn,
     CB_GETSTRING *namefn,
     CB_GETMOVE *getmovefn,
     CB_ISLEGAL *islegalfn,
     const char *pri_or_sec)
 {
     // If a library is already loaded at this address, unload it first
     // This simplistic approach assumes the caller manages pointers correctly
     if (*lib_ptr && (*lib_ptr)->isLoaded()) {
         qDebug() << "Unloading existing library for" << pri_or_sec << "engine before loading new one.";
         (*lib_ptr)->unload();
         delete *lib_ptr;
         *lib_ptr = nullptr;
     }

     // Construct path using Qt
     // Assume CBdirectory holds the base path as a C-string
     QString engineDir = QDir(QString::fromUtf8(CBdirectory)).filePath("engines");
     QString libPath = QDir(engineDir).filePath(QString::fromUtf8(dllname));

     qInfo() << "Attempting to load" << pri_or_sec << "engine from:" << libPath;

     *lib_ptr = new QLibrary(libPath);

     if (!(*lib_ptr)->load()) {
         qWarning() << "CheckerBoard could not find or load the" << pri_or_sec << "engine library:" << (*lib_ptr)->errorString();
         qWarning() << "Please use the 'Engine->Select..' command to select a new" << pri_or_sec << "engine.";

         delete *lib_ptr;
         *lib_ptr = nullptr;
         *cmdfn = NULL;
         *namefn = NULL;
         *getmovefn = NULL;
         *islegalfn = NULL;
         return (1); // Error
     }

     // If the handle is valid, try to get the function addresses
     qInfo() << "Successfully loaded" << pri_or_sec << "engine library:" << libPath;

     *cmdfn = (CB_ENGINECOMMAND)(*lib_ptr)->resolve("enginecommand");
     *namefn = (CB_GETSTRING)(*lib_ptr)->resolve("enginename");
     *getmovefn = (CB_GETMOVE)(*lib_ptr)->resolve("getmove");
     *islegalfn = (CB_ISLEGAL)(*lib_ptr)->resolve("islegal");

     // Check if essential functions were resolved
     if (!*getmovefn || !*cmdfn) {
          qCritical() << "Engine library" << dllname << "is missing required functions (getmove or enginecommand)!";
          (*lib_ptr)->unload();
          delete *lib_ptr;
          *lib_ptr = nullptr;
          *cmdfn = NULL;
          *namefn = NULL;
          *getmovefn = NULL;
          *islegalfn = NULL;
          return (1); // Error
     }


     // Fallback for optional islegal function
     if (*islegalfn == NULL) {
         qInfo() << "Engine does not provide 'islegal', using built-in English checker rules.";
         *islegalfn = builtinislegal; // builtinislegal needs to be accessible
     }

     qInfo() << "Resolved functions for" << pri_or_sec << "engine.";
     return (0); // Success
 }


 void loadengines(const char *pri_fname, const char *sec_fname)
 // sets the engines
 // This function needs significant rework in C++. It relies on global function pointers.
 {
     int status;
     // Temporary QLibrary pointers - these should be members of a C++ class
     static QLibrary *lib1 = nullptr;
     static QLibrary *lib2 = nullptr;

     qDebug() << "loadengines called with primary:" << pri_fname << "secondary:" << sec_fname;

     // Set built in functions (if needed globally)
     CBgametype = (CB_GETGAMETYPE)builtingametype;

     // --- Unloading Logic Simplified ---
     // We simply try to load the new ones. load_engine_qt handles unloading if the pointer exists.
     // More robust logic would compare filenames etc. and belong in a C++ manager class.

     // Load primary engine
     status = load_engine_qt(&lib1, pri_fname, &enginecommand1, &enginename1, &getmove1, &islegal1, "primary");
     if (!status) {
         // Successfully loaded new primary engine, update options if needed (careful with global cboptions)
         // This assumes cboptions is accessible and modifiable here. Risky in C context.
          // strncpy(cboptions.primaryenginestring, pri_fname, sizeof(cboptions.primaryenginestring) - 1);
          // cboptions.primaryenginestring[sizeof(cboptions.primaryenginestring) - 1] = '\0';
          qDebug() << "Primary engine loaded successfully:" << pri_fname;
     } else {
         qWarning() << "Failed to load primary engine:" << pri_fname << ". Attempting to reload previous.";
         // Try reloading the one stored in options if different
          // This requires cboptions to be accurately reflecting the *currently* loaded one, which might not be true.
         // if (strcmp(pri_fname, cboptions.primaryenginestring) != 0 && strlen(cboptions.primaryenginestring) > 0) {
         //     status = load_engine_qt(&lib1, cboptions.primaryenginestring, &enginecommand1, &enginename1, &getmove1, &islegal1, "primary (fallback)");
         //     if (status) cboptions.primaryenginestring[0] = '\0'; // Failed fallback too
         // } else {
              // cboptions.primaryenginestring[0] = '\0'; // Failed to load specified and it was the same as options, or options empty
         // }
          enginecommand1 = NULL; // Ensure null if load fails
          enginename1 = NULL;
          getmove1 = NULL;
          islegal1 = NULL;
     }


     // Load secondary engine
     status = load_engine_qt(&lib2, sec_fname, &enginecommand2, &enginename2, &getmove2, &islegal2, "secondary");
      if (!status) {
          // Successfully loaded new secondary engine
          // strncpy(cboptions.secondaryenginestring, sec_fname, sizeof(cboptions.secondaryenginestring) - 1);
          // cboptions.secondaryenginestring[sizeof(cboptions.secondaryenginestring) - 1] = '\0';
          qDebug() << "Secondary engine loaded successfully:" << sec_fname;

      } else {
         qWarning() << "Failed to load secondary engine:" << sec_fname << ". Attempting to reload previous.";
         // Try reloading the one stored in options if different
          // if (strcmp(sec_fname, cboptions.secondaryenginestring) != 0 && strlen(cboptions.secondaryenginestring) > 0) {
          //    status = load_engine_qt(&lib2, cboptions.secondaryenginestring, &enginecommand2, &enginename2, &getmove2, &islegal2, "secondary (fallback)");
          //    if (status) cboptions.secondaryenginestring[0] = '\0';
          // } else {
          //     cboptions.secondaryenginestring[0] = '\0';
          // }
          enginecommand2 = NULL; // Ensure null if load fails
          enginename2 = NULL;
          getmove2 = NULL;
          islegal2 = NULL;

     }


     // Set current engine (assuming engine 1 is default)
     setcurrentengine(1); // setcurrentengine needs access to the global function pointers

     // Reset game if an engine of different game type was selected!
     // This requires gametype() function to work correctly based on resolved pointers
     // And assumes cbgame is accessible globally.
     // if (gametype() != cbgame.gametype) {
     //    qDebug() << "Engine gametype changed, starting new game.";
     //    PostMessage(hwnd, (UINT)WM_COMMAND, (WPARAM)GAMENEW, (LPARAM)0); // Needs signal/slot
     //    PostMessage(hwnd, (UINT)WM_SIZE, (WPARAM)0, (LPARAM)0); // Needs signal/slot or direct call
     // }

     // Reset the directory? Avoid SetCurrentDirectory. Ensure paths used are absolute or relative to known base.
     // QDir::setCurrent(QString::fromUtf8(CBdirectory)); // Avoid if possible
     qDebug() << "loadengines finished.";
 }

 void initengines(void)
 {
     qDebug() << "initengines called.";
     // Assumes cboptions is initialized and accessible globally
     // loadengines(cboptions.primaryenginestring, cboptions.secondaryenginestring);
     // TEMPORARY: Need access to cboptions. For now, call with empty strings if not available.
     // Replace with proper access via MainWindow later.
     char default_pri[] = ""; // Default to empty if cboptions not available
     char default_sec[] = "";
     // TODO: Get actual engine names from cboptions safely
     loadengines(default_pri, default_sec);
 }

 // ... (rest of CheckerBoard.c) ...


