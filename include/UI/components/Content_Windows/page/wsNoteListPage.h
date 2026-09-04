//
// Created by DevAccount on 26/08/2026.
//

#ifndef TASKHELPER_WSNOTELISTPAGE_H
#define TASKHELPER_WSNOTELISTPAGE_H
#pragma once

#include <QQuickWidget>
#include <QDesktopServices>
#include <QUrl>
#include "../IWorkspaceView.h"
#include "helpers/Workspace.h"
#include "Data/workspace/Manager/NoteManager.h"

class wsNoteListPage : public IWorkspaceView
{

    Q_OBJECT
    // Context & Scope Properties
    Q_PROPERTY(QString workspaceName READ workspaceName NOTIFY workspaceChanged)
    Q_PROPERTY(QString projectName READ projectName NOTIFY projectChanged)
    Q_PROPERTY(QString projectColor READ projectColor NOTIFY projectChanged)
    Q_PROPERTY(bool isProjectScoped READ isProjectScoped NOTIFY scopeChanged)

    // Note Collection Properties
    Q_PROPERTY(QVariantList notes READ notes NOTIFY notesChanged)
    Q_PROPERTY(QVariantList pinnedNotes READ pinnedNotes NOTIFY pinnedNotesChanged)
    Q_PROPERTY(int totalNoteCount READ totalNoteCount NOTIFY notesChanged)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY notesChanged)

    // Search & Filter Properties
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(QString activeFilter READ activeFilter WRITE setActiveFilter NOTIFY filterChanged)
    Q_PROPERTY(QString sortMode READ sortMode WRITE setSortMode NOTIFY sortChanged)
    Q_PROPERTY(bool sortAscending READ sortAscending WRITE setSortAscending NOTIFY sortChanged)

    // View State Properties
    Q_PROPERTY(QString viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged) // "grid" | "list"
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)

    // Selection & Bulk Actions
    Q_PROPERTY(QStringList selectedNoteIds READ selectedNoteIds NOTIFY selectionChanged)
    Q_PROPERTY(bool isSelectionMode READ isSelectionMode WRITE setSelectionMode NOTIFY selectionModeChanged)


    

public:

    explicit wsNoteListPage(const Workspace& ws ,WorkspaceRepository* repo ,QWidget* parent = nullptr);

    // IWorkspaceView Overrides
    void refresh() override;
    void updateWorkspace(const Workspace& ws) override;
    void setActiveProject(const Project& project) override;

    QString workspaceName() const;
    QString projectName() const;
    QString projectColor() const;
    bool isProjectScoped() const;

    QVariantList notes() const;
    QVariantList pinnedNotes() const;
    int totalNoteCount() const;
    bool isEmpty() const;

    QString searchQuery() const;
    void setSearchQuery(const QString& query);

    QString activeFilter() const;
    void setActiveFilter(const QString& filter);

    QString sortMode() const;
    void setSortMode(const QString& mode);

    bool sortAscending() const;
    void setSortAscending(bool ascending);

    QString viewMode() const;
    void setViewMode(const QString& mode);

    bool isLoading() const;

    QStringList selectedNoteIds() const;
    bool isSelectionMode() const;
    void setSelectionMode(bool enabled);

    Q_INVOKABLE void createNewNote();
    Q_INVOKABLE void openNote(const QString& noteId);
    Q_INVOKABLE void togglePin(const QString& noteId);
    Q_INVOKABLE void deleteNote(const QString& noteId);
    Q_INVOKABLE void duplicateNote(const QString& noteId);
    Q_INVOKABLE void moveToProject(const QString& noteId, const QString& newProjectId);
    Q_INVOKABLE void bulkDelete(const QStringList& noteIds);
    Q_INVOKABLE void toggleSelectNote(const QString& noteId);
    Q_INVOKABLE void selectAllNotes();
    Q_INVOKABLE void clearSelection();

signals:
    void workspaceChanged();
    void projectChanged();
    void scopeChanged();
    void notesChanged();
    void pinnedNotesChanged();
    void searchQueryChanged();
    void filterChanged();
    void sortChanged();
    void viewModeChanged();
    void loadingChanged();
    void selectionChanged();
    void selectionModeChanged();

    void noteOpenRequested(const QString& noteId);


private:

    void setupUi();
    void populateData();
    QVariantMap noteToVariantMap(const Note& note) const;
    Workspace m_workspace;
    Project m_activeProject;
    WorkspaceRepository* m_repo{nullptr};
    NoteManager* m_noteManager{nullptr};
    QQuickWidget* m_quickWidget{nullptr};
    QUuid workspaceId_;

    // Internal State
    QVariantList m_notes;
    QVariantList m_pinnedNotes;
    QString m_searchQuery;
    QString m_activeFilter = "all";       // "all" | "pinned" | "recent" | "archived"
    QString m_sortMode = "updatedAt";     // "updatedAt" | "createdAt" | "title"
    bool m_sortAscending = false;
    QString m_viewMode = "grid";          // "grid" | "list"
    bool m_isLoading = false;
    QStringList m_selectedNoteIds;
    bool m_isSelectionMode = false;

};

#endif //TASKHELPER_WSNOTELISTPAGE_H
