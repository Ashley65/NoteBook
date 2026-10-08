#include "helpers/WasmStorageHelper.h"
#include <QDebug>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void WasmStorageHelper::initStorage(const QString& mountPath)
{
#ifdef __EMSCRIPTEN__
    static bool s_mounted = false;
    if (s_mounted) return;
    s_mounted = true;

    EM_ASM({
        var path = UTF8ToString($0);
        try {
            FS.mkdirTree(path);
        } catch (e) {}

        try {
            FS.mount(IDBFS, {}, path);
            console.log("[WasmStorageHelper] Mounted IDBFS at: " + path);
        } catch (e) {
            console.log("[WasmStorageHelper] Mount notice:", e.message || e);
        }

        FS.syncfs(true, function(err) {
            if (err) {
                console.error("[WasmStorageHelper] Error populating from IDBFS:", err);
            } else {
                console.log("[WasmStorageHelper] Successfully populated filesystem from IndexedDB.");
            }
        });
    }, mountPath.toUtf8().constData());

    registerBeforeUnloadHandler();
#else
    Q_UNUSED(mountPath);
#endif
}

void WasmStorageHelper::syncToBrowser()
{
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (typeof FS !== 'undefined' && FS.syncfs) {
            FS.syncfs(false, function(err) {
                if (err) {
                    console.error("[WasmStorageHelper] Error saving to IDBFS:", err);
                } else {
                    console.log("[WasmStorageHelper] Data synced to browser IndexedDB.");
                }
            });
        }
    });
#endif
}

void WasmStorageHelper::registerBeforeUnloadHandler()
{
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (typeof window !== 'undefined' && !window.__taskhelper_unload_registered) {
            window.__taskhelper_unload_registered = true;
            window.addEventListener('beforeunload', function() {
                if (typeof FS !== 'undefined' && FS.syncfs) {
                    FS.syncfs(false, function() {});
                }
            });
        }
    });
#endif
}
