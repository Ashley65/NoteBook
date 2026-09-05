//
// Created by DevAccount on 24/04/2026.
//

#include "Data/attachments/AttachmentValidator.h"
#include <QMimeDatabase>
#include <QMimeType>
#include <QFileInfo>

AttachmentValidator::AttachmentValidator()
    : m_maxFileSizeBytes(25 * 1024 * 1024) // Default 25 MB
{
}

void AttachmentValidator::setMaxFileSize(qint64 bytes) {
    m_maxFileSizeBytes = bytes;
}

void AttachmentValidator::setAllowedMimeTypes(const QSet<QString>& mimeTypes) {
    m_allowedMimeTypes = mimeTypes;
}

void AttachmentValidator::setAllowedExtensions(const QSet<QString>& extensionsLowercase) {
    m_allowedExtensions = extensionsLowercase;
}

AttachmentValidatorResult AttachmentValidator::validateLocalFile(const QString& sourceFilePath) const {
    AttachmentValidatorResult result;

    QFileInfo info(sourceFilePath);
    if (!info.exists()) {
        result.isValid = false;
        result.error = "File does not exist: " + sourceFilePath;
        return result;
    }

    if (!info.isFile()) {
        result.isValid = false;
        result.error = "Specified path is not a regular file: " + sourceFilePath;
        return result;
    }

    // Check file size
    if (m_maxFileSizeBytes > 0 && info.size() > m_maxFileSizeBytes) {
        result.isValid = false;
        result.error = QString("File size (%1 MB) exceeds maximum allowed size (%2 MB)")
            .arg(info.size() / (1024.0 * 1024.0), 0, 'f', 2)
            .arg(m_maxFileSizeBytes / (1024.0 * 1024.0), 0, 'f', 2);
        return result;
    }

    // Check extension if restricted
    QString ext = info.suffix().toLower();
    if (!m_allowedExtensions.isEmpty() && !m_allowedExtensions.contains(ext)) {
        result.isValid = false;
        result.error = QString("File extension '.%1' is not in the allowed list").arg(ext);
        return result;
    }

    // Detect MIME type
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(info);
    result.detectedMimeType = mimeType.name();

    if (!m_allowedMimeTypes.isEmpty() && !m_allowedMimeTypes.contains(result.detectedMimeType)) {
        result.isValid = false;
        result.error = QString("MIME type '%1' is not allowed").arg(result.detectedMimeType);
        return result;
    }

    result.isValid = true;
    return result;
}
