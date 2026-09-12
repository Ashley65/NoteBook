//
// Created by DevAccount on 22/05/2026.
//

#ifndef TASKHELPER_WSNOTEPAGE_H
#define TASKHELPER_WSNOTEPAGE_H
#pragma once

#include <QQuickWidget>
#include <QDesktopServices>
#include <QUrl>
#include "../IWorkspaceView.h"
#include "helpers/Workspace.h"
#include "Data/workspace/Manager/NoteManager.h"
#include "MD4C/mdAPI.h"


class wsNotePage : public IWorkspaceView
{
    Q_OBJECT

    // General Workspace Properties
    Q_PROPERTY(QString workspaceName READ workspaceName NOTIFY workspaceNameChanged)
    Q_PROPERTY(QVariantList recentNotes READ recentNotes NOTIFY recentNotesChanged)

    // Note editor state Properties
    Q_PROPERTY(QString currentNoteId READ currentNoteId NOTIFY currentNoteIdChanged)
    Q_PROPERTY(QString currentNoteTitle READ currentNoteTitle NOTIFY currentNoteTitleChanged)
    Q_PROPERTY(QString currentNoteContent READ currentNoteContent NOTIFY currentNoteContentChanged)
    Q_PROPERTY(QString saveStatus READ saveStatus NOTIFY saveStatusChanged)
    Q_PROPERTY(QString projectName READ projectName NOTIFY projectChanged)
    Q_PROPERTY(QString projectColor READ projectColor NOTIFY projectChanged)
    Q_PROPERTY(int wordCount READ wordCount NOTIFY noteStatsChanged)
    Q_PROPERTY(int characterCount READ characterCount NOTIFY noteStatsChanged)
    Q_PROPERTY(int readingTimeMinutes READ readingTimeMinutes NOTIFY noteStatsChanged)

    Q_PROPERTY(QVariantList linkedMentions READ linkedMentions NOTIFY linkedMentionsChanged)

public:

    explicit wsNotePage(const Workspace& ws ,WorkspaceRepository* repo ,QWidget* parent = nullptr);

    // IWorkspaceView Overrides
    void refresh() override;
    void updateWorkspace(const Workspace& ws) override;
    void setActiveProject(const Project& project) override;

    // QML Invokable for Editor Interaction
    Q_INVOKABLE void requestNewDraft();
    Q_INVOKABLE void commitDraftToDatabase(const QString& initialContent);
    Q_INVOKABLE void updateExistingNote(const QString& noteId, const QString& content);
    Q_INVOKABLE void updateNoteTitle(const QString& noteId, const QString& newTitle);
    Q_INVOKABLE void loadNote(const QString& noteId);
    Q_INVOKABLE QString renderMarkdownToHtml(const QString& markdown);
    Q_INVOKABLE void onLinkClicked(const QString& link);
    Q_INVOKABLE QVariantList searchNotesByTitle(const QString& query);
    Q_INVOKABLE QString attachFileToCurrentNote(const QString& localFilePath);
    Q_INVOKABLE void openAttachmentDialog();
    Q_INVOKABLE QVariantList searchAttachments(const QString& query);

    // Getters for QML Properties
    QString workspaceName() const;
    QString projectName() const;
    QString projectColor() const;
    int wordCount() const { return m_wordCount; }
    int characterCount() const { return m_characterCount; }
    int readingTimeMinutes() const { return m_readingTimeMinutes; }
    QVariantList recentNotes() const;
    QString currentNoteId() const;
    QString currentNoteTitle() const;
    QString currentNoteContent() const;
    QString saveStatus() const;
    QVariantList linkedMentions() const;


signals:
    void workspaceNameChanged();
    void projectChanged();
    void noteStatsChanged();
    void recentNotesChanged();
    void currentNoteIdChanged();
    void currentNoteTitleChanged();
    void currentNoteContentChanged();
    void saveStatusChanged();
    void draftCommitted(const QString& newId);
    void linkedMentionsChanged();

private:
    Workspace m_workspace;
    Project m_activeProject;
    WorkspaceRepository* m_repo {nullptr};
    NoteManager* m_noteManager {nullptr};

    QQuickWidget* m_noteQuickView {nullptr};

    QVariantList m_recentNotes;
    QString m_currentNoteId = "-1";
    QString m_currentNoteTitle = "";
    QString m_currentNoteContent = "";
    QString m_saveStatus = "State: In-Memory Draft (Unsaved)";
    QVariantList m_linkedMentions;

    int m_wordCount = 0;
    int m_characterCount = 0;
    int m_readingTimeMinutes = 1;

    void setupUi();
    void populateData();
    void updateStats(const QString& content);

};

#endif //TASKHELPER_WSNOTEPAGE_H
