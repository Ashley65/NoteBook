//
// Created by DevAccount on 24/04/2026.
//

#ifndef TASKHELPER_ATTACHMENTVALIDATOR_H
#define TASKHELPER_ATTACHMENTVALIDATOR_H
#pragma once
#include <QString>
#include <QFileInfo>
#include <QSet>

struct AttachmentValidatorResult
{
    bool isValid = false;
    QString error;
    QString detectedMimeType;
};


class AttachmentValidator
{
public:
    AttachmentValidator();

    void setMaxFileSize(qint64 bytes);
    void setAllowedMimeTypes(const QSet<QString>& mimeTypes);
    void setAllowedExtensions(const QSet<QString>& extensionsLowercase);

    AttachmentValidatorResult validateLocalFile(const QString& sourceFilePath) const;


private:
    qint64 m_maxFileSizeBytes;
    QSet<QString> m_allowedMimeTypes;
    QSet<QString> m_allowedExtensions;


};
#endif //TASKHELPER_ATTACHMENTVALIDATOR_H
