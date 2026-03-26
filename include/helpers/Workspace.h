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
    QString id;
    QString name;
    QString type;
    QString description;

    // Appearance
    QString icon;
    QColor color;

    // State
    bool protectedMode;
    bool isArchived;
    bool isPinned;

    // Ordering
    int order;

    // Timestamps,
    QDateTime createdAt;
    QDateTime updatedAt;
    QDateTime lastOpenedAt;

    // Cached counts for performance
    int taskCount;              // cached
    int noteCount;              // cached
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
