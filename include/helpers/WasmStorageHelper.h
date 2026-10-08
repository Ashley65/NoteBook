#ifndef TASKHELPER_WASMSTORAGEHELPER_H
#define TASKHELPER_WASMSTORAGEHELPER_H

#include <QString>

class WasmStorageHelper
{
public:
    // Mount IDBFS at data path and perform initial sync from IndexedDB to MEMFS
    static void initStorage(const QString& mountPath);

    // Flush changes from MEMFS to browser IndexedDB
    static void syncToBrowser();

    // Register browser beforeunload event listener to save pending writes on tab close
    static void registerBeforeUnloadHandler();
};

#endif // TASKHELPER_WASMSTORAGEHELPER_H
