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
    QString id; // Used to uniquely identify the workspace, allowing for reliable referencing and management of workspaces within the application.
    QString name; // Used to provide a human-readable identifier for the workspace, making it easier for users to recognise and differentiate between multiple workspaces in the UI.
    QString type; // e.g., "default", "custom", etc.
    QString description; // Used to provide additional information about the workspace, which can be displayed in the UI to help users understand the purpose or contents of the workspace at a glance.

    QString icon;
    QColor color;

    bool protectedMode; // Used to determine if the workspace should be protected from accidental deletion or modification, providing an extra layer of safety for important workspaces.
    bool isArchived; // Used to determine if the workspace should be hidden from the main workspace list in the UI, allowing users to keep old or unused workspaces without cluttering their main view.
    bool isPinned; // Used to determine if the workspace should be displayed at the top of the list in the UI, allowing users to quickly access their most important workspaces.

    int order; // Used to determine the order of workspaces in the UI, allowing users to customise their workspace arrangement.

    QDateTime createdAt; // Used to determine when the workspace was created, which can be useful for sorting or displaying in the UI.
    QDateTime updatedAt; // Used to determine when the workspace was last modified, which can be useful for sorting or displaying in the UI.
    QDateTime lastOpenedAt; // Used to tell the program how to sort the workspaces in the UI, the most recently opened workspace should be at the top of the list.

    int taskCount;              // cached
    int noteCount;              // cached
};

enum class WorkspaceType
{
    Default, // Used to identify the default workspace created when the application is first launched. This workspace can be used as a starting point for users to organise their tasks and notes, and it can also serve as a fallback option if users accidentally delete or modify their other workspaces.
    Custom, // Used to identify user-created workspaces that can be customised with different names, descriptions, icons, and colours. This allows users to create workspaces that suit their specific needs and preferences.
    System // Used to identify workspaces that are reserved for system use, such as a "Trash" workspace for deleted items or an "Archive" workspace for completed tasks. These workspaces can have special behaviours and restrictions to prevent users from accidentally modifying or deleting important system data.
};


#endif //TASKHELPER_WORKSPACE_H
