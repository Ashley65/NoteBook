//
// Created by DevAccount on 29/01/2026.
//

#ifndef TASKHELPER_WORKSPACECOMMAND_H
#define TASKHELPER_WORKSPACECOMMAND_H
#pragma once
#include <QMetaType>

enum class WorkspaceCommand
{
    Switch,
    Settings,
    Create
};

Q_DECLARE_METATYPE(WorkspaceCommand)

#endif //TASKHELPER_WORKSPACECOMMAND_H