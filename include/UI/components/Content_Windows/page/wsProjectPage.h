#ifndef TASKHELPER_WSPROJECTPAGE_H
#define TASKHELPER_WSPROJECTPAGE_H
#pragma once

#include <QQuickWidget>
#include <QVariantList>
#include "../IWorkspaceView.h"
#include "helpers/Workspace.h"
#include "Data/workspace/Structure/Project.h"
#include "Data/workspace/Manager/NoteManager.h"
#include "Data/workspace/Manager/TaskManager.h"
#include "UI/components/Dialogs/UploadDialog.h"

class wsProjectPage : public IWorkspaceView
{
    Q_OBJECT

    // Project Identity & Meta
    Q_PROPERTY(QString workspaceName READ workspaceName NOTIFY projectChanged)
    Q_PROPERTY(QString projectId READ projectId NOTIFY projectChanged)
    Q_PROPERTY(QString projectName READ projectName NOTIFY projectChanged)
    Q_PROPERTY(QString projectColor READ projectColor NOTIFY projectChanged)
    Q_PROPERTY(QString projectInitials READ projectInitials NOTIFY projectChanged)
    Q_PROPERTY(QString projectDescription READ projectDescription NOTIFY projectChanged)
    Q_PROPERTY(QString createdAtFormatted READ createdAtFormatted NOTIFY projectChanged)

    // Progress & Metrics
    Q_PROPERTY(int totalTasksCount READ totalTasksCount NOTIFY statsChanged)
    Q_PROPERTY(int completedTasksCount READ completedTasksCount NOTIFY statsChanged)
    Q_PROPERTY(int inProgressTasksCount READ inProgressTasksCount NOTIFY statsChanged)
    Q_PROPERTY(int pendingTasksCount READ pendingTasksCount NOTIFY statsChanged)
    Q_PROPERTY(int completionPercentage READ completionPercentage NOTIFY statsChanged)
    Q_PROPERTY(int notesCount READ notesCount NOTIFY statsChanged)
    Q_PROPERTY(int filesCount READ filesCount NOTIFY statsChanged)

    // Collections
    Q_PROPERTY(QVariantList tasks READ tasks NOTIFY tasksChanged)
    Q_PROPERTY(QVariantList notes READ notes NOTIFY notesChanged)
    Q_PROPERTY(QVariantList attachments READ attachments NOTIFY attachmentsChanged)

public:
    explicit wsProjectPage(const Workspace& ws, const Project& project, WorkspaceRepository* repo, QWidget* parent = nullptr);
    explicit wsProjectPage(const Workspace& ws, WorkspaceRepository* repo, QWidget* parent = nullptr);
    ~wsProjectPage() override = default;

    void refresh() override;
    void updateWorkspace(const Workspace& ws) override;
    void setActiveProject(const Project& project) override;

    // Quick Actions
    Q_INVOKABLE void createNewTask(const QString& title, const QString& priority = "Medium");
    Q_INVOKABLE void createNewNote(const QString& title);
    Q_INVOKABLE void toggleTaskCompletion(const QString& taskId, bool completed);
    Q_INVOKABLE void deleteTask(const QString& taskId);

    Q_INVOKABLE void openNote(const QString& noteId);
    Q_INVOKABLE void deleteNote(const QString& noteId);
    Q_INVOKABLE void togglePinNote(const QString& noteId);

    Q_INVOKABLE void openUploadDialog();
    Q_INVOKABLE void openAttachment(const QString& attachmentId);
    Q_INVOKABLE void deleteAttachment(const QString& attachmentId);

    // Getters
    QString workspaceName() const;
    QString projectId() const;
    QString projectName() const;
    QString projectColor() const;
    QString projectInitials() const;
    QString projectDescription() const;
    QString createdAtFormatted() const;

    int totalTasksCount() const { return m_totalTasksCount; }
    int completedTasksCount() const { return m_completedTasksCount; }
    int inProgressTasksCount() const { return m_inProgressTasksCount; }
    int pendingTasksCount() const { return m_pendingTasksCount; }
    int completionPercentage() const { return m_completionPercentage; }
    int notesCount() const { return m_notesCount; }
    int filesCount() const { return m_filesCount; }

    QVariantList tasks() const { return m_tasks; }
    QVariantList notes() const { return m_notes; }
    QVariantList attachments() const { return m_attachments; }

signals:
    void projectChanged();
    void tasksChanged();
    void notesChanged();
    void attachmentsChanged();
    void statsChanged();
    void noteOpenRequested(const QString& noteId);
    void uploadMessage(const QString& message, bool isError);

private slots:
    void onUploadRequested(const QStringList& sourcePaths, const QUuid& workspaceId, const QUuid& projectId);

private:
    Workspace m_workspace;
    Project m_project;
    WorkspaceRepository* m_repo {nullptr};
    TaskManager* m_taskManager {nullptr};
    NoteManager* m_noteManager {nullptr};
    UploadDialog* m_uploadDialog {nullptr};
    QQuickWidget* m_quickView {nullptr};

    QVariantList m_tasks;
    QVariantList m_notes;
    QVariantList m_attachments;

    int m_totalTasksCount {0};
    int m_completedTasksCount {0};
    int m_inProgressTasksCount {0};
    int m_pendingTasksCount {0};
    int m_completionPercentage {0};
    int m_notesCount {0};
    int m_filesCount {0};

    void setupUi();
    void populateData();
};

#endif //TASKHELPER_WSPROJECTPAGE_H
