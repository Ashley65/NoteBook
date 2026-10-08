#ifndef TASKHELPER_SYNCMANAGER_H
#define TASKHELPER_SYNCMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QUuid>
#include <QTimer>

class WorkspaceRepository;
class PocketBaseClient;

class SyncManager : public QObject
{
    Q_OBJECT

public:
    enum class SyncState
    {
        Idle,
        Syncing,
        Success,
        Error
    };
    Q_ENUM(SyncState)

    explicit SyncManager(WorkspaceRepository* repository,
                         PocketBaseClient* client,
                         QObject* parent = nullptr);
    ~SyncManager() override = default;

    SyncState state() const;
    QString lastError() const;
    QDateTime lastSyncTime() const;

    // Trigger full two-way synchronization
    void syncNow();

signals:
    void syncStarted();
    void syncFinished(bool success, const QString& message);
    void syncStatusChanged(SyncState state, const QString& message);
    void lastSyncTimeChanged(const QDateTime& time);

private slots:
    void onAuthStateChanged(bool loggedIn);

private:
    void loadMappings();
    void saveMappings();

    void stepPullWorkspaces();
    void stepPullProjects();
    void stepPullTasks();
    void stepPullNotes();
    void stepPushWorkspaces();
    void stepPushProjects();
    void stepPushTasks();
    void stepPushNotes();
    void completeSync(bool success, const QString& message = QString());

    WorkspaceRepository* m_repo{nullptr};
    PocketBaseClient* m_client{nullptr};

    SyncState m_state{SyncState::Idle};
    QString m_lastError;
    QDateTime m_lastSyncTime;

    QTimer* m_periodicTimer{nullptr};

    // ID Mappings between Local QUuid and PocketBase 15-char string ID
    QHash<QUuid, QString> m_localToRemoteWs;
    QHash<QString, QUuid> m_remoteToLocalWs;

    QHash<QUuid, QString> m_localToRemoteProj;
    QHash<QString, QUuid> m_remoteToLocalProj;

    QHash<QUuid, QString> m_localToRemoteTask;
    QHash<QString, QUuid> m_remoteToLocalTask;

    QHash<QUuid, QString> m_localToRemoteNote;
    QHash<QString, QUuid> m_remoteToLocalNote;

    bool m_isSyncing{false};
};

#endif // TASKHELPER_SYNCMANAGER_H
