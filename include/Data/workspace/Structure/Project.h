//
// Created by DevAccount on 31/03/2026.
//

#ifndef TASKHELPER_PROJECT_H
#define TASKHELPER_PROJECT_H
#pragma once

#include <QString>
#include <QDateTime>
#include <QUuid>

struct Project
{
    QUuid id;
    QUuid workspaceId;

    QString name;
    QString description;

    bool isArchived;

    QDateTime createdAt;
    QDateTime updatedAt;
};

#endif //TASKHELPER_PROJECT_H

