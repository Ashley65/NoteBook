

#ifndef TASKHELPER_NOTEATTACHMENTRENDERER_H
#define TASKHELPER_NOTEATTACHMENTRENDERER_H
#pragma once

#include <QString>
#include <QList>
#include <QFileInfo>
#include "Data/workspace/Structure/FileAttachment.h"

class NoteAttachmentRenderer
{
public:
    enum class FileType {
        Image,
        Pdf,
        Code,
        Csv,
        Generic
    };

    static FileType detectFileType(const QString& fileName);

    // Formats raw markdown by replacing ![[filename]] tags with rich HTML cards
    static QString processNoteMarkdown(const QString& rawMarkdown,
                                      const QList<FileAttachment>& availableAttachments);

    // Individual card renderers
    static QString renderImageCard(const QString& filePath, const QString& fileName, qint64 fileSize);
    static QString renderPdfCard(const QString& filePath, const QString& fileName, qint64 fileSize);
    static QString renderCodeCard(const QString& filePath, const QString& fileName, qint64 fileSize);
    static QString renderCsvCard(const QString& filePath, const QString& fileName, qint64 fileSize);
    static QString renderGenericCard(const QString& filePath, const QString& fileName, qint64 fileSize);
    static QString renderMissingCard(const QString& fileName);

    static QString formatFileSize(qint64 bytes);
};

#endif // TASKHELPER_NOTEATTACHMENTRENDERER_H
