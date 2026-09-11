//
// Created by DevAccount on 10/09/2026.
//

#ifndef TASKHELPER_DATASCHEMA_H
#define TASKHELPER_DATASCHEMA_H
#pragma once

#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QColor>
#include <QList>
#include <optional>
#include <vector>

#include "helpers/Workspace.h"
#include "Data/workspace/Structure/Project.h"
#include "Data/workspace/Structure/Task.h"
#include "Data/workspace/Structure/Note.h"
#include "Data/workspace/Structure/FileAttachment.h"

// Convenient aliases for database layer
using Subtask = SubTask;
using Attachment = FileAttachment;

#endif //TASKHELPER_DATASCHEMA_H
