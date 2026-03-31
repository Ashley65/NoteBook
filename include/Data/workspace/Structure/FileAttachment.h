//
// Created by DevAccount on 26/02/2026.
//

#ifndef TASKHELPER_FILEATTACHMENT_H
#define TASKHELPER_FILEATTACHMENT_H
#pragma once
#include <QString>
#include <QDateTime>
#include <QUuid>

struct FileAttachment
{
    QUuid id;
    QUuid workspaceId;
    QUuid projectId;
    QUuid noteId;      // optional (nullable)

    QString fileName;
    QString filePath;
    QString mimeType;
    qint64 fileSize;

    QDateTime createdAt;
};
#endif //TASKHELPER_FILEATTACHMENT_H