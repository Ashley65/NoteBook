//
// Created by DevAccount on 14/03/2026.
//

#ifndef TASKHELPER_VALIDATER_H
#define TASKHELPER_VALIDATER_H
#pragma once

#include <QRegularExpression>
#include <QString>

#include "Data/workspace/WorkspaceRepository.h"

struct NameValidationResult {
    bool ok;
    QString message;
};

static NameValidationResult validateWorkspaceName(const QString& name,
    const WorkspaceRepository* repo,
    const QUuid& excludeWorkspaceId = QUuid())
{
    static const QRegularExpression invalidChars(QStringLiteral("[\\\\/:*?\"<>|]"));
    const QString trimmed = name.trimmed();

    if (trimmed.isEmpty()) return {false, "Name cannot be empty."};
    if (trimmed.contains(invalidChars)) return {false, "Name contains invalid characters."};

    if (repo) {
        for (const auto& ws : repo->workspaces()) {
            if (!excludeWorkspaceId.isNull() && ws.id == excludeWorkspaceId) continue;
            if (ws.name.trimmed().compare(trimmed, Qt::CaseInsensitive) == 0) {
                return {false, "Workspace name already exists."};
            }
        }
    }
    return {true, {}};

};

#endif //TASKHELPER_VALIDATER_H