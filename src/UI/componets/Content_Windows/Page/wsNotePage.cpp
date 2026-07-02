//
// Created by DevAccount on 24/06/2026.
//
#include "UI/components/Content_Windows/page/wsNotePage.h"
#include "MD4C/mdAPI.h"
#include <QVBoxLayout>
#include <QQmlContext>
#include <QUuid>

wsNotePage::wsNotePage(const Workspace& ws, WorkspaceRepository* repo, QWidget* parent)
    : IWorkspaceView(ws, parent), m_workspace(ws), m_activeProject(), m_repo(repo)
{
    if (!m_repo)
    {
        return;
    }

    m_noteManager = new NoteManager(m_repo, this);

    connect(m_noteManager, &NoteManager::notesChanged, this, [this](const QUuid& wsId)
    {
        if (wsId == m_workspace.id)
        {
            populateData();
        }
    });


    // Initialise the UI and QML engine integration
    setupUi();

    // Load initial data for the sidebar
    populateData();
}

void wsNotePage::refresh()
{
    populateData();
}

void wsNotePage::updateWorkspace(const Workspace& ws)
{
    m_workspace = ws;
    emit workspaceNameChanged();
    refresh();
}

void wsNotePage::setActiveProject(const Project& project)
{
    IWorkspaceView::setActiveProject(project);
    m_activeProject = project;
    refresh();
}

void wsNotePage::requestNewDraft()
{
    m_currentNoteId = "-1";
    m_currentNoteTitle = "";
    m_currentNoteContent = "";
    m_saveStatus = "State: In-Memory Draft (Unsaved)";

    emit currentNoteIdChanged();
    emit currentNoteTitleChanged();
    emit currentNoteContentChanged();
    emit saveStatusChanged();
}

void wsNotePage::commitDraftToDatabase(const QString& initialContent)
{
    if (!m_noteManager)
    {
        return;
    }

    QString autoTitle = initialContent.left(20).simplified();
    if (autoTitle.isEmpty())
    {
        autoTitle = "Untitled Note";
    }
    // Prepare create request
    NoteCreateRequest request;
    request.title = autoTitle;
    request.content = initialContent;
    request.workspaceId = m_workspace.id;
    request.projectId = m_activeProject.id; // Repository will handle null IDs with defaults

    // Create the note via NoteManager
    QUuid newId = m_noteManager->createNote(request);

    if (!newId.isNull()) {
        m_currentNoteId = newId.toString(QUuid::WithoutBraces);
        m_currentNoteTitle = autoTitle;
        m_currentNoteContent = initialContent;
        m_saveStatus = "Saved to Disk";

        emit currentNoteIdChanged();
        emit currentNoteTitleChanged();
        emit currentNoteContentChanged();
        emit saveStatusChanged();
        emit draftCommitted(m_currentNoteId);

        // populateData() is called via the notesChanged signal connection
    }
}

void wsNotePage::updateExistingNote(const QString& noteId, const QString& content)
{
    if (noteId == "-1" || !m_noteManager) return;

    QUuid id = QUuid::fromString(noteId);
    Note note = m_noteManager->getNoteById(id);
    if (note.id.isNull()) return;

    note.content = content;
    m_noteManager->updateNote(note);

    m_currentNoteContent = content;
    m_saveStatus = "Last edited: Just now";

    QStringList extractedLinks = mdAPI::extractMentions(content);
    m_linkedMentions.clear();
    for (const QString& link : extractedLinks) {
        m_linkedMentions.append(link);
    }
    emit linkedMentionsChanged();

    emit currentNoteContentChanged();
    emit saveStatusChanged();
}

void wsNotePage::updateNoteTitle(const QString& noteId, const QString& newTitle)
{
    m_currentNoteTitle = newTitle;
    emit currentNoteTitleChanged();

    if (noteId == "-1" || !m_noteManager) return;

    QUuid id = QUuid::fromString(noteId);
    Note note = m_noteManager->getNoteById(id);
    if (note.id.isNull()) return;

    note.title = newTitle;
    m_noteManager->updateNote(note);

    m_currentNoteTitle = newTitle;
    emit currentNoteTitleChanged();
}

void wsNotePage::loadNote(const QString& noteId)
{
    if (!m_noteManager) return;

    QUuid id = QUuid::fromString(noteId);
    Note note = m_noteManager->getNoteById(id);

    if (!note.id.isNull()) {
        m_currentNoteId = note.id.toString(QUuid::WithoutBraces);
        m_currentNoteTitle = note.title;
        m_currentNoteContent = note.content;
        m_saveStatus = "State: Saved";

        QStringList extractedLinks = mdAPI::extractMentions(note.content);
        m_linkedMentions.clear();
        for (const QString& link : extractedLinks) {
            m_linkedMentions.append(link);
        }
        emit linkedMentionsChanged();
        emit currentNoteIdChanged();
        emit currentNoteTitleChanged();
        emit currentNoteContentChanged();
        emit saveStatusChanged();

    } else {
        qWarning() << "Failed to load note with ID:" << noteId;
    }





}

QString wsNotePage::renderMarkdownToHtml(const QString& markdown)
{
    return mdAPI::renderHtml(markdown);
}

void wsNotePage::onLinkClicked(const QString& link)
{
    // 1. Standard Web Links
    if (link.startsWith("http://") || link.startsWith("https://")) {
        QDesktopServices::openUrl(QUrl(link));
        return;
    }
    // 2. Local File System Paths
    else if (link.startsWith("file://") || link.startsWith("/")) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(link));
        return;
    }

    // 3. WikiLinks and Internal Notes
    qDebug() << "Raw Link clicked:" << link;

    // THE FIX: Decode the URL encoding (converts %5B to [, %20 to space, etc.)
    QString cleanLink = QUrl::fromPercentEncoding(link.toUtf8());

    // Clean up the "note://" prefix if the parser added it
    if (cleanLink.startsWith("note://")) {
        cleanLink = cleanLink.mid(7);
    }

    // Clean up "[[" and "]]" if the parser included the brackets in the href
    if (cleanLink.startsWith("[[")) {
        cleanLink = cleanLink.mid(2);
    }
    if (cleanLink.endsWith("]]")) {
        cleanLink = cleanLink.chopped(2);
    }

    qDebug() << "Processed Note Title:" << cleanLink;

    if (m_noteManager && !m_workspace.id.isNull()) {
        QList<Note> notes = m_noteManager->recentNotes(m_workspace.id);
        for (const auto& note : notes) {
            if (note.title.compare(cleanLink, Qt::CaseInsensitive) == 0) {
                loadNote(note.id.toString(QUuid::WithoutBraces));
                return;
            }
        }
    }

    qWarning() << "Could not find a note matching the title:" << cleanLink;
}

QVariantList wsNotePage::searchNotesByTitle(const QString& query)
{
    QVariantList results;
    if (!m_noteManager || m_workspace.id.isNull()) return results;

    QList<Note> notes = m_noteManager->recentNotes(m_workspace.id);

    for (const auto& note : notes)
    {
        if (note.title.contains(query, Qt::CaseInsensitive)) {
            QVariantMap map;
            map["id"] = note.id.toString(QUuid::WithoutBraces);
            map["title"] = note.title;
            map["preview"] = note.preview;
            map["updatedAt"] = note.updatedAt.isValid() ? note.updatedAt : note.createdAt;
            results.append(map);
        }
    }
    return results;
}

QString wsNotePage::workspaceName() const { return m_workspace.name; }
QVariantList wsNotePage::recentNotes() const { return m_recentNotes; }
QString wsNotePage::currentNoteId() const { return m_currentNoteId; }
QString wsNotePage::currentNoteTitle() const { return m_currentNoteTitle; }
QString wsNotePage::currentNoteContent() const { return m_currentNoteContent; }
QString wsNotePage::saveStatus() const { return m_saveStatus; }

QVariantList wsNotePage::linkedMentions() const { return m_linkedMentions; }


void wsNotePage::setupUi()
{

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_noteQuickView = new QQuickWidget(this);
    m_noteQuickView->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_noteQuickView->rootContext()->setContextProperty("notePageContext", this);

    // Attempt to load the QML
    m_noteQuickView->setSource(QUrl(QStringLiteral("qrc:/qml/wsNoteEditorLinker.qml")));

    // ADD THIS TO CATCH THE WHITE SCREEN ERROR:
    if (m_noteQuickView->status() == QQuickWidget::Error) {
        for (const QQmlError &error : m_noteQuickView->errors()) {
            qWarning() << "QML Error:" << error.toString();
        }
    }

    layout->addWidget(m_noteQuickView);
}

void wsNotePage::populateData()
{
    if (!m_noteManager || m_workspace.id.isNull()) return;

    m_recentNotes.clear();
    QList<Note> notes = m_noteManager->recentNotes(m_workspace.id);

    for (const auto& note : notes) {
        QVariantMap map;
        map["id"] = note.id.toString(QUuid::WithoutBraces);
        map["title"] = note.title;
        map["preview"] = note.preview;
        map["updatedAt"] = note.updatedAt.isValid() ? note.updatedAt : note.createdAt;
        m_recentNotes.append(map);
    }

    emit recentNotesChanged();
}
