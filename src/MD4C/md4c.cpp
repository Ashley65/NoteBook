#include "MD4C/mdAPI.h"

struct ParserState {
    QStringList linkedMentions;
    bool isInsideWikiLink = false;
    QString currentOutput;
};

QStringList mdAPI::extractMentions(const QString& markdown) {
    ParserState state;

    // Call the internal parsing engine
    parse(markdown, &state);

    // The state object has now been populated by the callbacks
    return state.linkedMentions;
}

int mdAPI::parse(const QString& text, ParserState* state) {
    // Configure the parser to look for WikiLinks
    MD_PARSER parser = {
        0, // abi_version
        MD_FLAG_WIKILINKS, // flags
        enter_block_callback,
        leave_block_callback,
        enter_span_callback,
        leave_span_callback,
        text_callback,
        nullptr, // debug_log
        nullptr  // syntax
    };

    QByteArray utf8Data = text.toUtf8();
    return md_parse(utf8Data.constData(), utf8Data.size(), &parser, state);
}

static void process_html_output(const MD_CHAR* text, MD_SIZE size, void* userdata) {
    QByteArray* html = static_cast<QByteArray*>(userdata);
    html->append(text, size);
}

QString mdAPI::renderHtml(const QString& markdown) {
    QByteArray utf8Data = markdown.toUtf8();
    QByteArray htmlOutput;

    // Enable advanced features: Tables, Task Lists, Strikethrough, WikiLinks, and disable raw HTML
    unsigned parser_flags = MD_FLAG_TABLES | MD_FLAG_TASKLISTS | MD_FLAG_STRIKETHROUGH | MD_FLAG_WIKILINKS | MD_FLAG_NOHTML;

    // Run the HTML converter
    md_html(utf8Data.constData(), utf8Data.size(), process_html_output, &htmlOutput, parser_flags, 0);

    // Inject custom CSS to style the Qt RichText elements!
    QString finalHtml = QString("<style>"
                                "table { border-collapse: collapse; width: 100%; } "
                                "th, td { border: 1px solid #555555; padding: 8px; } "
                                "th { background-color: #2D2D2D; } "
                                "code { background-color: #2D2D2D; font-family: Consolas, monospace; } "
                                "blockquote { margin: 0 0 0 10px; color: #858585; }"
                                "</style>") + QString::fromUtf8(htmlOutput);

    return finalHtml;
}
// --- MD4C Callbacks ---

int mdAPI::enter_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata) {
    // We don't need block-level data for mentions, but we must provide the callback
    return 0;
}

int mdAPI::leave_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata) {
    return 0;
}

int mdAPI::enter_span_callback(MD_SPANTYPE type, void* detail, void* userdata) {
    ParserState* state = static_cast<ParserState*>(userdata);

    if (type == MD_SPAN_WIKILINK) {
        state->isInsideWikiLink = true;
    }
    return 0;
}

int mdAPI::leave_span_callback(MD_SPANTYPE type, void* detail, void* userdata) {
    ParserState* state = static_cast<ParserState*>(userdata);

    if (type == MD_SPAN_WIKILINK) {
        state->isInsideWikiLink = false;
    }
    return 0;
}

int mdAPI::text_callback(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* userdata) {
    ParserState* state = static_cast<ParserState*>(userdata);

 
    if (state->isInsideWikiLink) {
        QString linkTarget = QString::fromUtf8(text, size);
        state->linkedMentions.append(linkTarget);
    }

    // Populate the currentOutput for potential future HTML conversion
    state->currentOutput.append(QString::fromUtf8(text, size));

    return 0;
}