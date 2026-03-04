//
// Created by DevAccount on 26/02/2026.
//

#ifndef TASKHELPER_NOTE_H
#define TASKHELPER_NOTE_H
#pragma once
#include <QString>
#include <QDateTime>
#include <QUuid>

struct Note
{
    QUuid id;
    QUuid workspaceId;

    QString title;
    QString content;       // Rich text (Markdown)
    QString preview;       // First few lines (cached)

    bool isPinned;
    bool isArchived;

    QDateTime createdAt;
    QDateTime updatedAt;
};



#endif //TASKHELPER_NOTE_H