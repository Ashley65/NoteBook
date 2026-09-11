//
// Created by DevAccount on 28/01/2026.
//

#ifndef TASKHELPER_WORKSPACE_H
#define TASKHELPER_WORKSPACE_H

#include <QString>
#include <QDateTime>
#include <QColor>
#include <QUuid>

struct Workspace
{
    // Core properties
    QUuid id;
    QString name;
    QString type;
    QString description;

    // Appearance
    QString icon;
    QColor color;

    // State
    bool protectedMode = false;
    bool isArchived = false;
    bool isPinned = false;

    // Ordering
    int order = 0;

    // Timestamps,
    QDateTime createdAt;
    QDateTime updatedAt;
    QDateTime lastOpenedAt;

    // Cached counts for performance
    int taskCount = 0;              // cached
    int noteCount = 0;              // cached
};

// Predefined workspace types for categorisation and UI purposes
enum class WorkspaceType
{
    Default,
    Work,
    Study,
    Lab,
    Custom,
    System
};


#endif //TASKHELPER_WORKSPACE_H
