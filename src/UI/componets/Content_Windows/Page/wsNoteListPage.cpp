//
// Created by DevAccount on 04/09/2026.
//

#include "UI/components/Content_Windows/page/wsNoteListPage.h"
#include <QVBoxLayout>
#include <QQmlContext>
#include <QRegularExpression>
#include <algorithm>

wsNoteListPage::wsNoteListPage(const Workspace& ws, WorkspaceRepository* repo, QWidget* parent)
    : IWorkspaceView(ws, parent), m_workspace(ws), m_repo(repo), workspaceId_(ws.id)
{
    if (!m_repo) {
        return;
    }

    m_noteManager = new NoteManager(m_repo, this);

    connect(m_noteManager, &NoteManager::notesChanged, this, [this](const QUuid& wsId) {
        if (wsId == workspaceId_ || wsId.isNull()) {
            populateData();
        }
    });

    setupUi();
    populateData();
}

void wsNoteListPage::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_quickWidget->setClearColor(Qt::transparent);

    m_quickWidget->rootContext()->setContextProperty("noteListPage", this);
    m_quickWidget->setSource(QUrl("qrc:/qml/wsNoteListLinker.qml"));

    if (m_quickWidget->status() == QQuickWidget::Error) {
        const auto errs = m_quickWidget->errors();
        for (const auto& err : errs) {
            qWarning() << "wsNoteListPage QML Error:" << err.toString();
        }
    }

    layout->addWidget(m_quickWidget, 1);
}

void wsNoteListPage::refresh()
{
    populateData();
}

void wsNoteListPage::updateWorkspace(const Workspace& ws)
{
    m_workspace = ws;
    workspaceId_ = ws.id;
    if (!m_activeProject.id.isNull()) {
        m_activeProject = Project{};
        emit projectChanged();
        emit scopeChanged();
    }
    emit workspaceChanged();
    refresh();
}

void wsNoteListPage::setActiveProject(const Project& project)
{
    IWorkspaceView::setActiveProject(project);
    m_activeProject = project;
    emit projectChanged();
    emit scopeChanged();
    refresh();
}

static QString colorForProject(const QUuid& projectId)
{
    if (projectId.isNull()) return "#81C784";
    static const QStringList palette = {
        "#81C784", "#FFD700", "#9ACD32", "#20B2AA", "#FF69B4", "#64B5F6", "#BA68C8", "#FF8A65"
    };
    return palette.at(qAbs(qHash(projectId.toString())) % palette.size());
}

QString wsNoteListPage::workspaceName() const
{
    return m_workspace.name;
}

QString wsNoteListPage::projectName() const
{
    return m_activeProject.name;
}

QString wsNoteListPage::projectColor() const
{
    return colorForProject(m_activeProject.id);
}

bool wsNoteListPage::isProjectScoped() const
{
    return !m_activeProject.id.isNull();
}

QVariantList wsNoteListPage::notes() const
{
    return m_notes;
}

QVariantList wsNoteListPage::pinnedNotes() const
{
    return m_pinnedNotes;
}

int wsNoteListPage::totalNoteCount() const
{
    return m_notes.count();
}

bool wsNoteListPage::isEmpty() const
{
    return m_notes.isEmpty();
}

QString wsNoteListPage::searchQuery() const
{
    return m_searchQuery;
}

void wsNoteListPage::setSearchQuery(const QString& query)
{
    if (m_searchQuery != query) {
        m_searchQuery = query;
        emit searchQueryChanged();
        populateData();
    }
}

QString wsNoteListPage::activeFilter() const
{
    return m_activeFilter;
}

void wsNoteListPage::setActiveFilter(const QString& filter)
{
    if (m_activeFilter != filter) {
        m_activeFilter = filter;
        emit filterChanged();
        populateData();
    }
}

QString wsNoteListPage::sortMode() const
{
    return m_sortMode;
}

void wsNoteListPage::setSortMode(const QString& mode)
{
    if (m_sortMode != mode) {
        m_sortMode = mode;
        emit sortChanged();
        populateData();
    }
}

bool wsNoteListPage::sortAscending() const
{
    return m_sortAscending;
}

void wsNoteListPage::setSortAscending(bool ascending)
{
    if (m_sortAscending != ascending) {
        m_sortAscending = ascending;
        emit sortChanged();
        populateData();
    }
}

QString wsNoteListPage::viewMode() const
{
    return m_viewMode;
}

void wsNoteListPage::setViewMode(const QString& mode)
{
    if (m_viewMode != mode) {
        m_viewMode = mode;
        emit viewModeChanged();
    }
}

bool wsNoteListPage::isLoading() const
{
    return m_isLoading;
}

QStringList wsNoteListPage::selectedNoteIds() const
{
    return m_selectedNoteIds;
}

bool wsNoteListPage::isSelectionMode() const
{
    return m_isSelectionMode;
}

void wsNoteListPage::setSelectionMode(bool enabled)
{
    if (m_isSelectionMode != enabled) {
        m_isSelectionMode = enabled;
        if (!enabled) {
            m_selectedNoteIds.clear();
            emit selectionChanged();
        }
        emit selectionModeChanged();
        populateData();
    }
}

void wsNoteListPage::createNewNote()
{
    if (!m_noteManager) return;

    NoteCreateRequest req;
    req.title = tr("Untitled Note");
    req.content = "";
    req.workspaceId = m_workspace.id;
    req.projectId = m_activeProject.id;
    req.isPinned = (m_activeFilter == "pinned");

    const QUuid newId = m_noteManager->createNote(req);
    if (!newId.isNull()) {
        const QString idStr = newId.toString(QUuid::WithoutBraces);
        emit noteOpenRequested(idStr);
    }
}

void wsNoteListPage::openNote(const QString& noteId)
{
    if (!noteId.isEmpty()) {
        emit noteOpenRequested(noteId);
    }
}

void wsNoteListPage::togglePin(const QString& noteId)
{
    if (!m_noteManager) return;
    const QUuid id = QUuid::fromString(noteId);
    if (id.isNull()) return;

    const Note note = m_noteManager->getNoteById(id);
    if (!note.id.isNull()) {
        m_noteManager->setPinned(id, !note.isPinned);
    }
}

void wsNoteListPage::deleteNote(const QString& noteId)
{
    if (!m_noteManager) return;
    const QUuid id = QUuid::fromString(noteId);
    if (!id.isNull()) {
        m_noteManager->deleteNote(id);
    }
}

void wsNoteListPage::duplicateNote(const QString& noteId)
{
    if (!m_noteManager) return;
    const QUuid id = QUuid::fromString(noteId);
    if (id.isNull()) return;

    const Note note = m_noteManager->getNoteById(id);
    if (note.id.isNull()) return;

    NoteCreateRequest req;
    req.title = QString("%1 (Copy)").arg(note.title);
    req.content = note.content;
    req.workspaceId = note.workspaceId;
    req.projectId = note.projectId;
    req.isPinned = note.isPinned;

    m_noteManager->createNote(req);
}

void wsNoteListPage::moveToProject(const QString& noteId, const QString& newProjectId)
{
    if (!m_noteManager) return;
    const QUuid id = QUuid::fromString(noteId);
    if (id.isNull()) return;

    Note note = m_noteManager->getNoteById(id);
    if (note.id.isNull()) return;

    note.projectId = QUuid::fromString(newProjectId);
    m_noteManager->updateNote(note);
}

void wsNoteListPage::bulkDelete(const QStringList& noteIds)
{
    if (!m_noteManager) return;
    for (const QString& idStr : noteIds) {
        const QUuid id = QUuid::fromString(idStr);
        if (!id.isNull()) {
            m_noteManager->deleteNote(id);
        }
    }
    clearSelection();
}

void wsNoteListPage::toggleSelectNote(const QString& noteId)
{
    if (noteId.isEmpty()) return;
    if (m_selectedNoteIds.contains(noteId)) {
        m_selectedNoteIds.removeAll(noteId);
    } else {
        m_selectedNoteIds.append(noteId);
    }
    emit selectionChanged();
    populateData();
}

void wsNoteListPage::selectAllNotes()
{
    m_selectedNoteIds.clear();
    for (const auto& item : m_notes) {
        const auto map = item.toMap();
        m_selectedNoteIds.append(map["id"].toString());
    }
    emit selectionChanged();
    populateData();
}

void wsNoteListPage::clearSelection()
{
    m_selectedNoteIds.clear();
    emit selectionChanged();
    populateData();
}

QVariantMap wsNoteListPage::noteToVariantMap(const Note& note) const
{
    QVariantMap map;
    map["id"] = note.id.toString(QUuid::WithoutBraces);
    map["title"] = note.title.isEmpty() ? tr("Untitled Note") : note.title;

    QString previewText = note.preview;
    if (previewText.isEmpty() && !note.content.isEmpty()) {
        previewText = note.content.left(160).trimmed();
        previewText.replace('\n', ' ');
    }
    map["preview"] = previewText;

    // Word count calculation
    int words = 0;
    if (!note.content.trimmed().isEmpty()) {
        words = note.content.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    }
    QString wordCountStr = words >= 1000
        ? QString("%1k words").arg(QString::number(words / 1000.0, 'f', 1))
        : QString("%1 words").arg(words);
    map["wordCountFormatted"] = wordCountStr;

    // Relative Date Formatter
    const QDateTime now = QDateTime::currentDateTime();
    const qint64 secs = note.updatedAt.secsTo(now);
    QString timeStr;
    if (secs < 60) {
        timeStr = tr("Just now");
    } else if (secs < 3600) {
        timeStr = tr("%1m ago").arg(secs / 60);
    } else if (secs < 86400) {
        timeStr = tr("%1h ago").arg(secs / 3600);
    } else if (secs < 86400 * 2) {
        timeStr = tr("Yesterday");
    } else if (secs < 86400 * 7) {
        timeStr = tr("%1 days ago").arg(secs / 86400);
    } else {
        timeStr = note.updatedAt.toString("MMM d, yyyy");
    }
    map["updatedAtFormatted"] = timeStr;
    map["isPinned"] = note.isPinned;
    map["isArchived"] = note.isArchived;
    map["projectId"] = note.projectId.toString(QUuid::WithoutBraces);

    // Project metadata
    if (!note.projectId.isNull() && m_repo) {
        const Project p = m_repo->getProjectById(note.projectId);
        map["projectName"] = p.name;
        map["projectColor"] = colorForProject(note.projectId);
    } else {
        map["projectName"] = "";
        map["projectColor"] = "#81C784";
    }

    map["isSelected"] = m_selectedNoteIds.contains(map["id"].toString());
    return map;
}

void wsNoteListPage::populateData()
{
    if (!m_repo) return;

    m_isLoading = true;
    emit loadingChanged();

    QList<Note> allSourceNotes;
    if (!m_activeProject.id.isNull()) {
        allSourceNotes = m_repo->getNotesByProject(m_activeProject.id);
    } else if (!workspaceId_.isNull()) {
        allSourceNotes = m_repo->getNotesByWorkspace(workspaceId_);
    }

    // Filter by search query and activeFilter
    QList<Note> filtered;
    for (const auto& note : allSourceNotes) {
        if (!m_searchQuery.trimmed().isEmpty()) {
            const QString q = m_searchQuery.trimmed().toLower();
            const bool matchTitle = note.title.toLower().contains(q);
            const bool matchContent = note.content.toLower().contains(q);
            if (!matchTitle && !matchContent) {
                continue;
            }
        }

        if (m_activeFilter == "pinned" && !note.isPinned) continue;
        if (m_activeFilter == "archived" && !note.isArchived) continue;
        if (m_activeFilter != "archived" && note.isArchived) continue;
        if (m_activeFilter == "recent") {
            if (note.updatedAt.daysTo(QDateTime::currentDateTime()) > 7) continue;
        }

        filtered.append(note);
    }

    // Sort
    std::sort(filtered.begin(), filtered.end(), [this](const Note& a, const Note& b) {
        bool less = false;
        if (m_sortMode == "title") {
            less = QString::compare(a.title, b.title, Qt::CaseInsensitive) < 0;
        } else if (m_sortMode == "createdAt") {
            less = a.createdAt < b.createdAt;
        } else {
            less = a.updatedAt < b.updatedAt;
        }
        return m_sortAscending ? less : !less;
    });

    m_notes.clear();
    m_pinnedNotes.clear();

    for (const auto& note : filtered) {
        const QVariantMap map = noteToVariantMap(note);
        m_notes.append(map);
        if (note.isPinned) {
            m_pinnedNotes.append(map);
        }
    }

    m_isLoading = false;
    emit loadingChanged();
    emit notesChanged();
    emit pinnedNotesChanged();
}
