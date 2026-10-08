#ifndef TASKHELPER_WASMCLIPBOARDHELPER_H
#define TASKHELPER_WASMCLIPBOARDHELPER_H

#include <QString>

class WasmClipboardHelper
{
public:
    // Initialize browser DOM clipboard event bridges (paste / copy)
    static void initClipboard();

    // Read clipboard text from browser / Qt
    static QString getBrowserClipboardText();

    // Set clipboard text in browser / Qt
    static void setBrowserClipboardText(const QString& text);
};

#endif // TASKHELPER_WASMCLIPBOARDHELPER_H
