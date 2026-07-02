
#ifndef MD4C_MDC_H
#define MD4C_MDC_H
#pragma once

#include <md4c.h>
#include <QString>
#include <QStringList>
#include <md4c.h>
#include <md4c-html.h>


struct ParserState;

class mdAPI
{
public:
    static QStringList extractMentions(const QString& markdown);

    static int parse(const QString& text, ParserState* state);

    static QString renderHtml(const QString& markdown);

private:
    // Callback functions for md4c parser
    static int enter_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata);
    static int leave_block_callback(MD_BLOCKTYPE type, void* detail, void* userdata);
    static int enter_span_callback(MD_SPANTYPE type, void* detail, void* userdata);
    static int leave_span_callback(MD_SPANTYPE type, void* detail, void* userdata);
    static int text_callback(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* userdata);
};





#endif // MD4C_MDC_H
