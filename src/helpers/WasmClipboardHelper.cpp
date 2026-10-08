#include "helpers/WasmClipboardHelper.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QApplication>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QDebug>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/val.h>

static QString s_lastWasmClipboardText;

extern "C" {

EMSCRIPTEN_KEEPALIVE void flow_wasm_on_paste(const char* utf8Text)
{
    if (!utf8Text) return;
    QString text = QString::fromUtf8(utf8Text);
    s_lastWasmClipboardText = text;

    if (QGuiApplication::clipboard()) {
        QGuiApplication::clipboard()->setText(text);
    }

    QWidget* fw = QApplication::focusWidget();
    if (fw) {
        if (auto* le = qobject_cast<QLineEdit*>(fw)) {
            le->insert(text);
        } else if (auto* te = qobject_cast<QTextEdit*>(fw)) {
            te->insertPlainText(text);
        } else if (auto* pte = qobject_cast<QPlainTextEdit*>(fw)) {
            pte->insertPlainText(text);
        }
    }
}

EMSCRIPTEN_KEEPALIVE const char* flow_wasm_get_clipboard()
{
    static QByteArray s_buf;
    if (QGuiApplication::clipboard()) {
        s_buf = QGuiApplication::clipboard()->text().toUtf8();
        return s_buf.constData();
    }
    return "";
}

} // extern "C"
#endif

void WasmClipboardHelper::initClipboard()
{
#ifdef __EMSCRIPTEN__
    static bool s_initialized = false;
    if (s_initialized) return;
    s_initialized = true;

    EM_ASM({
        if (typeof window !== 'undefined' && !window.__flow_clipboard_listener_installed) {
            window.__flow_clipboard_listener_installed = true;

            // Bridge browser 'paste' event (Ctrl+V / Cmd+V / browser context menu)
            window.addEventListener('paste', function(e) {
                var text = (e.clipboardData || window.clipboardData).getData('text');
                if (text && text.length > 0) {
                    if (typeof _flow_wasm_on_paste === 'function') {
                        var ptr = stringToNewUTF8(text);
                        _flow_wasm_on_paste(ptr);
                        _free(ptr);
                    } else if (typeof Module !== 'undefined' && Module._flow_wasm_on_paste) {
                        var ptr = stringToNewUTF8(text);
                        Module._flow_wasm_on_paste(ptr);
                        _free(ptr);
                    }
                }
            }, true);

            // Bridge browser 'copy' event (Ctrl+C / Cmd+C)
            window.addEventListener('copy', function(e) {
                var text = "";
                if (typeof _flow_wasm_get_clipboard === 'function') {
                    var ptr = _flow_wasm_get_clipboard();
                    if (ptr) text = UTF8ToString(ptr);
                } else if (typeof Module !== 'undefined' && Module._flow_wasm_get_clipboard) {
                    var ptr = Module._flow_wasm_get_clipboard();
                    if (ptr) text = UTF8ToString(ptr);
                }
                if (text && text.length > 0 && e.clipboardData) {
                    e.clipboardData.setData('text/plain', text);
                }
            }, true);

            console.log("[WasmClipboardHelper] Browser clipboard bridge registered.");
        }
    });
#endif
}

QString WasmClipboardHelper::getBrowserClipboardText()
{
#ifdef __EMSCRIPTEN__
    emscripten::val navigator = emscripten::val::global("navigator");
    if (!navigator.isUndefined() && !navigator["clipboard"].isUndefined()) {
        // Reads or prompts for browser clipboard
    }
    if (!s_lastWasmClipboardText.isEmpty()) {
        return s_lastWasmClipboardText;
    }
#endif
    if (QGuiApplication::clipboard()) {
        return QGuiApplication::clipboard()->text();
    }
    return QString();
}

void WasmClipboardHelper::setBrowserClipboardText(const QString& text)
{
    if (QGuiApplication::clipboard()) {
        QGuiApplication::clipboard()->setText(text);
    }
#ifdef __EMSCRIPTEN__
    s_lastWasmClipboardText = text;
    EM_ASM({
        var text = UTF8ToString($0);
        if (navigator && navigator.clipboard && navigator.clipboard.writeText) {
            navigator.clipboard.writeText(text).catch(function(err) {
                console.warn("[WasmClipboardHelper] writeText warning:", err);
            });
        }
    }, text.toUtf8().constData());
#endif
}
