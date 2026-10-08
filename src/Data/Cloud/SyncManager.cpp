#include "Data/Cloud/SyncManager.h"
#include "Data/Cloud/PocketBaseClient.h"
#include "Data/workspace/WorkspaceRepository.h"

#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

SyncManager::SyncManager(WorkspaceRepository* repository,
                         PocketBaseClient* client,
                         QObject* parent)
    : QObject(parent)
    , m_repo(repository)
    , m_client(client)
    , m_periodicTimer(new QTimer(this))
{
    loadMappings();

    connect(m_client, &PocketBaseClient::authStateChanged, this, &SyncManager::onAuthStateChanged);

    // Periodic sync every 3 minutes
    connect(m_periodicTimer, &QTimer::timeout, this, [this]() {
        if (m_client && m_client->isLoggedIn()) {
            syncNow();
        }
    });
    m_periodicTimer->start(180000); // 3 minutes
}

SyncManager::SyncState SyncManager::state() const
{
    return m_state;
}

QString SyncManager::lastError() const
{
    return m_lastError;
}

QDateTime SyncManager::lastSyncTime() const
{
    return m_lastSyncTime;
}

void SyncManager::onAuthStateChanged(bool loggedIn)
{
    if (loggedIn) {
        syncNow();
    } else {
        m_state = SyncState::Idle;
        emit syncStatusChanged(m_state, tr("Logged out"));
    }
}

void SyncManager::syncNow()
{
    if (!m_client || !m_client->isLoggedIn()) {
        completeSync(false, tr("Not connected to a cloud provider."));
        return;
    }

    if (m_isSyncing) {
        return;
    }

    m_isSyncing = true;
    m_state = SyncState::Syncing;
    emit syncStarted();
    emit syncStatusChanged(m_state, tr("Syncing data with cloud..."));

    stepPullWorkspaces();
}

void SyncManager::stepPullWorkspaces()
{
    m_client->fetchRecords("workspaces", QString(), [this](bool success, const QJsonArray& items, const QString& error) {
        if (!success) {
            completeSync(false, tr("Failed to fetch workspaces: %1").arg(error));
            return;
        }

        for (const auto& val : items) {
            const QJsonObject obj = val.toObject();
            const QString remoteId = obj.value("id").toString();
            const QString name = obj.value("name").toString();
            const QString type = obj.value("type").toString();
            const QString description = obj.value("description").toString();
            const QString icon = obj.value("icon").toString();
            const QString color = obj.value("color").toString();
            const bool isPinned = obj.value("isPinned").toBool();
            const bool isArchived = obj.value("isArchived").toBool();
            const bool protectedMode = obj.value("protectedMode").toBool();
            const int order = obj.value("order").toInt();

            QUuid localId = m_remoteToLocalWs.value(remoteId);
            if (localId.isNull()) {
                // Check if local workspace exists with same name
                bool found = false;
                for (const auto& ws : m_repo->workspaces()) {
                    if (ws.name.compare(name, Qt::CaseInsensitive) == 0) {
                        localId = ws.id;
                        m_localToRemoteWs[localId] = remoteId;
                        m_remoteToLocalWs[remoteId] = localId;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    localId = m_repo->createWorkspace(name, type, description);
                    m_localToRemoteWs[localId] = remoteId;
                    m_remoteToLocalWs[remoteId] = localId;
                }
            }

            Workspace ws = m_repo->getWorkspaceById(localId);
            if (!ws.id.isNull()) {
                ws.name = name;
                ws.type = type;
                ws.description = description;
                ws.icon = icon;
                if (!color.isEmpty()) ws.color = QColor(color);
                ws.isPinned = isPinned;
                ws.isArchived = isArchived;
                ws.protectedMode = protectedMode;
                ws.order = order;
                m_repo->updateWorkspace(ws);
            }
        }

        saveMappings();
        stepPullProjects();
    });
}

void SyncManager::stepPullProjects()
{
    m_client->fetchRecords("projects", QString(), [this](bool success, const QJsonArray& items, const QString& error) {
        if (!success) {
            completeSync(false, tr("Failed to fetch projects: %1").arg(error));
            return;
        }

        for (const auto& val : items) {
            const QJsonObject obj = val.toObject();
            const QString remoteId = obj.value("id").toString();
            const QString remoteWsId = obj.value("workspace").toString();
            const QString name = obj.value("name").toString();
            const QString description = obj.value("description").toString();
            const bool isArchived = obj.value("isArchived").toBool();

            const QUuid localWsId = m_remoteToLocalWs.value(remoteWsId);
            if (localWsId.isNull()) continue;

            QUuid localId = m_remoteToLocalProj.value(remoteId);
            if (localId.isNull()) {
                for (const auto& p : m_repo->getProjectsByWorkspace(localWsId)) {
                    if (p.name.compare(name, Qt::CaseInsensitive) == 0) {
                        localId = p.id;
                        m_localToRemoteProj[localId] = remoteId;
                        m_remoteToLocalProj[remoteId] = localId;
                        break;
                    }
                }

                if (localId.isNull()) {
                    Project newProj;
                    newProj.workspaceId = localWsId;
                    newProj.name = name;
                    newProj.description = description;
                    newProj.isArchived = isArchived;
                    newProj.createdAt = QDateTime::currentDateTime();
                    newProj.updatedAt = newProj.createdAt;
                    localId = m_repo->createProject(newProj);
                    m_localToRemoteProj[localId] = remoteId;
                    m_remoteToLocalProj[remoteId] = localId;
                }
            }

            Project p = m_repo->getProjectById(localId);
            if (!p.id.isNull()) {
                p.name = name;
                p.description = description;
                p.isArchived = isArchived;
                m_repo->updateProject(p);
            }
        }

        saveMappings();
        stepPullTasks();
    });
}

void SyncManager::stepPullTasks()
{
    m_client->fetchRecords("tasks", QString(), [this](bool success, const QJsonArray& items, const QString& error) {
        if (!success) {
            completeSync(false, tr("Failed to fetch tasks: %1").arg(error));
            return;
        }

        for (const auto& val : items) {
            const QJsonObject obj = val.toObject();
            const QString remoteId = obj.value("id").toString();
            const QString remoteWsId = obj.value("workspace").toString();
            const QString remoteProjId = obj.value("project").toString();
            const QString title = obj.value("title").toString();
            const QString description = obj.value("description").toString();
            const int statusVal = obj.value("status").toString().toInt();
            const int priorityVal = obj.value("priority").toString().toInt();

            const QUuid localWsId = m_remoteToLocalWs.value(remoteWsId);
            if (localWsId.isNull()) continue;

            QUuid localProjId = m_remoteToLocalProj.value(remoteProjId);
            if (localProjId.isNull()) {
                const auto projs = m_repo->getProjectsByWorkspace(localWsId);
                if (!projs.isEmpty()) {
                    localProjId = projs.first().id;
                }
            }

            QUuid localId = m_remoteToLocalTask.value(remoteId);
            if (localId.isNull()) {
                Task newTask;
                newTask.workspaceId = localWsId;
                newTask.projectId = localProjId;
                newTask.title = title;
                newTask.description = description;
                newTask.status = static_cast<TaskStatus>(statusVal);
                newTask.priority = static_cast<TaskPriority>(priorityVal);
                newTask.createdAt = QDateTime::currentDateTime();
                localId = m_repo->createTask(newTask);
                m_localToRemoteTask[localId] = remoteId;
                m_remoteToLocalTask[remoteId] = localId;
            } else {
                Task t = m_repo->getTaskById(localId);
                if (!t.id.isNull()) {
                    t.title = title;
                    t.description = description;
                    t.status = static_cast<TaskStatus>(statusVal);
                    t.priority = static_cast<TaskPriority>(priorityVal);
                    m_repo->updateTask(t);
                }
            }
        }

        saveMappings();
        stepPullNotes();
    });
}

void SyncManager::stepPullNotes()
{
    m_client->fetchRecords("notes", QString(), [this](bool success, const QJsonArray& items, const QString& error) {
        if (!success) {
            completeSync(false, tr("Failed to fetch notes: %1").arg(error));
            return;
        }

        for (const auto& val : items) {
            const QJsonObject obj = val.toObject();
            const QString remoteId = obj.value("id").toString();
            const QString remoteWsId = obj.value("workspace").toString();
            const QString remoteProjId = obj.value("project").toString();
            const QString title = obj.value("title").toString();
            const QString content = obj.value("content").toString();
            const QString preview = obj.value("preview").toString();
            const bool isPinned = obj.value("isPinned").toBool();
            const bool isArchived = obj.value("isArchived").toBool();

            const QUuid localWsId = m_remoteToLocalWs.value(remoteWsId);
            if (localWsId.isNull()) continue;

            QUuid localProjId = m_remoteToLocalProj.value(remoteProjId);
            if (localProjId.isNull()) {
                const auto projs = m_repo->getProjectsByWorkspace(localWsId);
                if (!projs.isEmpty()) {
                    localProjId = projs.first().id;
                }
            }

            QUuid localId = m_remoteToLocalNote.value(remoteId);
            if (localId.isNull()) {
                Note newNote;
                newNote.workspaceId = localWsId;
                newNote.projectId = localProjId;
                newNote.title = title;
                newNote.content = content;
                newNote.preview = preview;
                newNote.isPinned = isPinned;
                newNote.isArchived = isArchived;
                newNote.createdAt = QDateTime::currentDateTime();
                newNote.updatedAt = newNote.createdAt;
                localId = m_repo->createNote(newNote);
                m_localToRemoteNote[localId] = remoteId;
                m_remoteToLocalNote[remoteId] = localId;
            }

            Note n = m_repo->getNoteById(localId);
            if (!n.id.isNull()) {
                n.title = title;
                n.content = content;
                n.preview = preview;
                n.isPinned = isPinned;
                n.isArchived = isArchived;
                m_repo->updateNote(n);
            }
        }

        saveMappings();
        stepPushWorkspaces();
    });
}

void SyncManager::stepPushWorkspaces()
{
    const auto workspaces = m_repo->workspaces();
    for (const auto& ws : workspaces) {
        if (!m_localToRemoteWs.contains(ws.id)) {
            QJsonObject payload;
            payload["name"] = ws.name;
            payload["type"] = ws.type;
            payload["description"] = ws.description;
            payload["icon"] = ws.icon;
            payload["color"] = ws.color.name();
            payload["isPinned"] = ws.isPinned;
            payload["isArchived"] = ws.isArchived;
            payload["protectedMode"] = ws.protectedMode;
            payload["order"] = ws.order;
            payload["user"] = m_client->userId();

            const QUuid localId = ws.id;
            m_client->createRecord("workspaces", payload, [this, localId](bool success, const QJsonObject& record, const QString&) {
                if (success) {
                    const QString remoteId = record.value("id").toString();
                    m_localToRemoteWs[localId] = remoteId;
                    m_remoteToLocalWs[remoteId] = localId;
                    saveMappings();
                }
            });
        }
    }

    stepPushProjects();
}

void SyncManager::stepPushProjects()
{
    for (const auto& ws : m_repo->workspaces()) {
        const QString remoteWsId = m_localToRemoteWs.value(ws.id);
        if (remoteWsId.isEmpty()) continue;

        for (const auto& p : m_repo->getProjectsByWorkspace(ws.id)) {
            if (!m_localToRemoteProj.contains(p.id)) {
                QJsonObject payload;
                payload["name"] = p.name;
                payload["description"] = p.description;
                payload["isArchived"] = p.isArchived;
                payload["workspace"] = remoteWsId;

                const QUuid localId = p.id;
                m_client->createRecord("projects", payload, [this, localId](bool success, const QJsonObject& record, const QString&) {
                    if (success) {
                        const QString remoteId = record.value("id").toString();
                        m_localToRemoteProj[localId] = remoteId;
                        m_remoteToLocalProj[remoteId] = localId;
                        saveMappings();
                    }
                });
            }
        }
    }

    stepPushTasks();
}

void SyncManager::stepPushTasks()
{
    for (const auto& ws : m_repo->workspaces()) {
        const QString remoteWsId = m_localToRemoteWs.value(ws.id);
        if (remoteWsId.isEmpty()) continue;

        for (const auto& t : m_repo->getTasksByWorkspace(ws.id)) {
            if (!m_localToRemoteTask.contains(t.id)) {
                QJsonObject payload;
                payload["title"] = t.title;
                payload["description"] = t.description;
                payload["status"] = QString::number(static_cast<int>(t.status));
                payload["priority"] = QString::number(static_cast<int>(t.priority));
                payload["workspace"] = remoteWsId;
                if (m_localToRemoteProj.contains(t.projectId)) {
                    payload["project"] = m_localToRemoteProj.value(t.projectId);
                }

                const QUuid localId = t.id;
                m_client->createRecord("tasks", payload, [this, localId](bool success, const QJsonObject& record, const QString&) {
                    if (success) {
                        const QString remoteId = record.value("id").toString();
                        m_localToRemoteTask[localId] = remoteId;
                        m_remoteToLocalTask[remoteId] = localId;
                        saveMappings();
                    }
                });
            }
        }
    }

    stepPushNotes();
}

void SyncManager::stepPushNotes()
{
    for (const auto& ws : m_repo->workspaces()) {
        const QString remoteWsId = m_localToRemoteWs.value(ws.id);
        if (remoteWsId.isEmpty()) continue;

        for (const auto& n : m_repo->getNotesByWorkspace(ws.id)) {
            if (!m_localToRemoteNote.contains(n.id)) {
                QJsonObject payload;
                payload["title"] = n.title;
                payload["content"] = n.content;
                payload["preview"] = n.preview;
                payload["isPinned"] = n.isPinned;
                payload["isArchived"] = n.isArchived;
                payload["workspace"] = remoteWsId;
                if (m_localToRemoteProj.contains(n.projectId)) {
                    payload["project"] = m_localToRemoteProj.value(n.projectId);
                }

                const QUuid localId = n.id;
                m_client->createRecord("notes", payload, [this, localId](bool success, const QJsonObject& record, const QString&) {
                    if (success) {
                        const QString remoteId = record.value("id").toString();
                        m_localToRemoteNote[localId] = remoteId;
                        m_remoteToLocalNote[remoteId] = localId;
                        saveMappings();
                    }
                });
            }
        }
    }

    completeSync(true);
}

void SyncManager::completeSync(bool success, const QString& message)
{
    m_isSyncing = false;
    if (success) {
        m_state = SyncState::Success;
        m_lastError.clear();
        m_lastSyncTime = QDateTime::currentDateTime();
        emit lastSyncTimeChanged(m_lastSyncTime);
        emit syncFinished(true, tr("Sync complete."));
        emit syncStatusChanged(m_state, tr("Sync complete (%1)").arg(m_lastSyncTime.toString("hh:mm:ss")));
    } else {
        m_state = SyncState::Error;
        m_lastError = message;
        emit syncFinished(false, message);
        emit syncStatusChanged(m_state, message);
    }
}

void SyncManager::loadMappings()
{
    QSettings settings;
    settings.beginGroup("SyncMappings");

    auto readMap = [&settings](const QString& groupName, QHash<QUuid, QString>& l2r, QHash<QString, QUuid>& r2l) {
        settings.beginGroup(groupName);
        for (const auto& key : settings.childKeys()) {
            const QUuid local = QUuid::fromString(key);
            const QString remote = settings.value(key).toString();
            if (!local.isNull() && !remote.isEmpty()) {
                l2r[local] = remote;
                r2l[remote] = local;
            }
        }
        settings.endGroup();
    };

    readMap("workspaces", m_localToRemoteWs, m_remoteToLocalWs);
    readMap("projects", m_localToRemoteProj, m_remoteToLocalProj);
    readMap("tasks", m_localToRemoteTask, m_remoteToLocalTask);
    readMap("notes", m_localToRemoteNote, m_remoteToLocalNote);

    settings.endGroup();
}

void SyncManager::saveMappings()
{
    QSettings settings;
    settings.beginGroup("SyncMappings");

    auto writeMap = [&settings](const QString& groupName, const QHash<QUuid, QString>& l2r) {
        settings.beginGroup(groupName);
        for (auto it = l2r.begin(); it != l2r.end(); ++it) {
            settings.setValue(it.key().toString(QUuid::WithoutBraces), it.value());
        }
        settings.endGroup();
    };

    writeMap("workspaces", m_localToRemoteWs);
    writeMap("projects", m_localToRemoteProj);
    writeMap("tasks", m_localToRemoteTask);
    writeMap("notes", m_localToRemoteNote);

    settings.endGroup();
}
