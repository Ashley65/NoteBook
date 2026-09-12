//
// Created by DevAccount on 24/06/2026.
//
#include "UI/components/Content_Windows/page/wsNotePage.h"
#include "MD4C/mdAPI.h"
#include "helpers/NoteAttachmentRenderer.h"
#include <QVBoxLayout>
#include <QQmlContext>
#include <QUuid>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include <QFileDialog>
#include <QRegularExpression>
#include <QFileInfo>

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

    updateStats("");
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
        m_saveStatus = "State: Saved";

        emit currentNoteIdChanged();
        emit currentNoteTitleChanged();
        emit currentNoteContentChanged();
        emit saveStatusChanged();
        emit draftCommitted(m_currentNoteId);

        updateStats(initialContent);

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
    m_saveStatus = "State: Saved";

    QStringList extractedLinks = mdAPI::extractMentions(content);
    m_linkedMentions.clear();
    for (const QString& link : extractedLinks) {
        m_linkedMentions.append(link);
    }
    emit linkedMentionsChanged();

    emit currentNoteContentChanged();
    emit saveStatusChanged();

    updateStats(content);
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

        if (m_repo && !note.projectId.isNull()) {
            m_activeProject = m_repo->getProjectById(note.projectId);
            emit projectChanged();
        }

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

        updateStats(m_currentNoteContent);
    } else {
        qWarning() << "Failed to load note with ID:" << noteId;
    }
}

QString wsNotePage::renderMarkdownToHtml(const QString& markdown)
{
    if (markdown.isEmpty()) return "";

    // 1. Gather available attachments for this note / project / workspace
    QList<FileAttachment> available;
    if (m_repo) {
        if (!m_activeProject.id.isNull()) {
            available.append(m_repo->getAttachmentsByProject(m_activeProject.id));
        }
        if (!m_workspace.id.isNull()) {
            const auto wsAtts = m_repo->getAttachmentsByWorkspace(m_workspace.id);
            for (const auto& a : wsAtts) {
                bool found = false;
                for (const auto& existing : available) {
                    if (existing.id == a.id) { found = true; break; }
                }
                if (!found) available.append(a);
            }
        }
    }

    // 2. Pre-process attachment tags and replace with unique tokens
    // We use tokens to protect HTML card markup from MD4C's NOHTML flag
    struct TokenCard {
        QString token;
        QString html;
    };
    QList<TokenCard> tokenCards;

    static const QRegularExpression wikiEmbedRegex(R"(!\[\[([^\]|]+)(?:\|([^\]]+))?\]\])");
    QString tokenizedMarkdown = markdown;
    QRegularExpressionMatchIterator it = wikiEmbedRegex.globalMatch(tokenizedMarkdown);

    struct MatchLoc {
        int start;
        int length;
        QString fileName;
    };
    QList<MatchLoc> matches;
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        MatchLoc loc;
        loc.start = m.capturedStart();
        loc.length = m.capturedLength();
        loc.fileName = m.captured(1).trimmed();
        matches.append(loc);
    }

    // Apply tokens backwards to preserve character indices
    for (int i = matches.size() - 1; i >= 0; --i) {
        const QString token = QString("CHRONOATTACHMENTTOKEN%1END").arg(i);

        // Find matching attachment
        FileAttachment targetAtt;
        for (const auto& a : available) {
            if (a.fileName.compare(matches[i].fileName, Qt::CaseInsensitive) == 0 ||
                QFileInfo(a.relativePath).fileName().compare(matches[i].fileName, Qt::CaseInsensitive) == 0) {
                targetAtt = a;
                break;
            }
        }
        if (targetAtt.relativePath.isEmpty() && QFile::exists(matches[i].fileName)) {
            targetAtt.fileName = QFileInfo(matches[i].fileName).fileName();
            targetAtt.relativePath = matches[i].fileName;
            targetAtt.fileSize = QFileInfo(matches[i].fileName).size();
        }

        QString cardHtml;
        if (targetAtt.relativePath.isEmpty() || !QFile::exists(targetAtt.relativePath)) {
            cardHtml = NoteAttachmentRenderer::renderMissingCard(matches[i].fileName);
        } else {
            const NoteAttachmentRenderer::FileType type = NoteAttachmentRenderer::detectFileType(
                targetAtt.fileName.isEmpty() ? matches[i].fileName : targetAtt.fileName);
            switch (type) {
                case NoteAttachmentRenderer::FileType::Image:
                    cardHtml = NoteAttachmentRenderer::renderImageCard(targetAtt.relativePath, targetAtt.fileName, targetAtt.fileSize);
                    break;
                case NoteAttachmentRenderer::FileType::Pdf:
                    cardHtml = NoteAttachmentRenderer::renderPdfCard(targetAtt.relativePath, targetAtt.fileName, targetAtt.fileSize);
                    break;
                case NoteAttachmentRenderer::FileType::Code:
                    cardHtml = NoteAttachmentRenderer::renderCodeCard(targetAtt.relativePath, targetAtt.fileName, targetAtt.fileSize);
                    break;
                case NoteAttachmentRenderer::FileType::Csv:
                    cardHtml = NoteAttachmentRenderer::renderCsvCard(targetAtt.relativePath, targetAtt.fileName, targetAtt.fileSize);
                    break;
                case NoteAttachmentRenderer::FileType::Generic:
                default:
                    cardHtml = NoteAttachmentRenderer::renderGenericCard(targetAtt.relativePath, targetAtt.fileName, targetAtt.fileSize);
                    break;
            }
        }

        TokenCard tc;
        tc.token = token;
        tc.html = cardHtml;
        tokenCards.append(tc);

        tokenizedMarkdown.replace(matches[i].start, matches[i].length, token);
    }

    // 3. Render markdown to HTML via MD4C with ChronoTasks dark theme styling
    static const QString chronoThemeCss =
        "<style>"
        "body { color: #E2E8F0; font-family: 'Segoe UI', system-ui, sans-serif; font-size: 15px; line-height: 1.65; }"
        "h1 { color: #FFFFFF; font-size: 22px; font-weight: 700; margin-top: 18px; margin-bottom: 8px; border-bottom: 1px solid rgba(255,255,255,0.08); padding-bottom: 6px; }"
        "h2 { color: #F1F5F9; font-size: 18px; font-weight: 600; margin-top: 16px; margin-bottom: 6px; }"
        "h3 { color: #E2E8F0; font-size: 16px; font-weight: 600; margin-top: 12px; margin-bottom: 4px; }"
        "p { margin: 0 0 12px 0; color: #CBD5E1; line-height: 1.65; }"
        "a { color: #818CF8; text-decoration: none; font-weight: 500; }"
        "ul, ol { margin-top: 4px; margin-bottom: 12px; padding-left: 22px; color: #CBD5E1; }"
        "li { margin-bottom: 4px; }"
        "blockquote { margin: 12px 0; padding: 6px 14px; background-color: #171926; border-left: 3px solid #8B5CF6; color: #94A3B8; border-radius: 0 6px 6px 0; }"
        "code { background-color: #1B1D2C; color: #E0E7FF; padding: 2px 6px; border-radius: 4px; font-family: Consolas, monospace; font-size: 13px; border: 1px solid rgba(255,255,255,0.06); }"
        "table { border-collapse: collapse; width: 100%; margin: 12px 0; border: 1px solid #2B2D3F; border-radius: 6px; }"
        "th { background-color: #1A1C2B; color: #93C5FD; padding: 8px 12px; border: 1px solid #2B2D3F; font-weight: 600; text-align: left; }"
        "td { padding: 8px 12px; border: 1px solid #2B2D3F; color: #CBD5E1; }"
        "hr { border: none; border-top: 1px solid rgba(255, 255, 255, 0.08); margin: 16px 0; }"
        "</style>";

    QString renderedHtml = chronoThemeCss + mdAPI::renderHtml(tokenizedMarkdown);

    // 3.5. Convert Markdown task list items into Qt RichText compatible checkbox rows
    // MD4C generates <li class="task-list-item"><input type="checkbox"...> which Qt ignores.
    // We convert them into styled Unicode ballot checkboxes (checked &#9745;, unchecked &#9744;) without bullet dots.
    static const QRegularExpression checkedTaskRegex(
        "<li\\s+class=\"task-list-item\">\\s*<input\\s+type=\"checkbox\"\\s+class=\"task-list-item-checkbox\"\\s+disabled\\s+checked\\s*>(.*?)</li>",
        QRegularExpression::DotMatchesEverythingOption
    );
    renderedHtml.replace(checkedTaskRegex,
        "<div style=\"margin: 4px 0 4px 16px; line-height: 1.6;\">"
        "<span style=\"color: #10B981; font-weight: bold; font-size: 15px;\">&#9745;</span>&nbsp;&nbsp;"
        "<span style=\"color: #94A3B8;\">\\1</span>"
        "</div>"
    );

    static const QRegularExpression uncheckedTaskRegex(
        "<li\\s+class=\"task-list-item\">\\s*<input\\s+type=\"checkbox\"\\s+class=\"task-list-item-checkbox\"\\s+disabled\\s*>(.*?)</li>",
        QRegularExpression::DotMatchesEverythingOption
    );
    renderedHtml.replace(uncheckedTaskRegex,
        "<div style=\"margin: 4px 0 4px 16px; line-height: 1.6;\">"
        "<span style=\"color: #64748B; font-weight: bold; font-size: 15px;\">&#9744;</span>&nbsp;&nbsp;"
        "<span style=\"color: #E2E8F0;\">\\1</span>"
        "</div>"
    );

    // Clean up surrounding <ul> and </ul> when they enclose task item <div>s
    static const QRegularExpression taskUlOpenRegex("<ul>\\s*(?=<div style=\"margin: 4px 0 4px 16px;)");
    renderedHtml.replace(taskUlOpenRegex, "<div style=\"margin: 6px 0 12px 0;\">");

    static const QRegularExpression taskUlCloseRegex("(?<=</div>)\\s*</ul>");
    renderedHtml.replace(taskUlCloseRegex, "</div>");

    // 4. Post-process: substitute tokens with the rich HTML cards
    for (const auto& tc : tokenCards) {
        // MD4C encloses standalone block-like tokens in <p>...</p> tags.
        // Nesting <table> or <div> inside <p> breaks QTextHtmlParser in Qt.
        // Therefore, replace any <p>-wrapped tokens first, then bare tokens:
        const QRegularExpression pWrappedToken(QString(R"(<p>\s*%1\s*</p>)").arg(QRegularExpression::escape(tc.token)));
        renderedHtml.replace(pWrappedToken, tc.html);
        renderedHtml.replace(QString("<p>%1").arg(tc.token), tc.html);
        renderedHtml.replace(tc.token, tc.html);
    }

    return renderedHtml;
}

void wsNotePage::onLinkClicked(const QString& link)
{
    QUrl parsedUrl(link);
    QString scheme = parsedUrl.scheme().toLower();

    // 1. Action links for embedded attachments
    if (scheme == "action") {
        QUrlQuery query(parsedUrl);
        QString filePath = query.queryItemValue("path");
        if (filePath.isEmpty() && query.hasQueryItem("file")) {
            filePath = query.queryItemValue("file");
        }
        filePath = QUrl::fromPercentEncoding(filePath.toUtf8());

        if (!filePath.isEmpty() && QFile::exists(filePath)) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        } else {
            qWarning() << "Attachment file not found for action:" << link << "filePath:" << filePath;
        }
        return;
    }

    // 2. Standard Web & Mail Links (Whitelisted safe external schemes)
    if (scheme == "http" || scheme == "https" || scheme == "mailto") {
        QDesktopServices::openUrl(parsedUrl);
        return;
    }

    // 3. Local file URLs
    if (scheme == "file") {
        QString localPath = parsedUrl.toLocalFile();
        if (QFile::exists(localPath)) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(localPath));
        }
        return;
    }

    // 4. WikiLinks and Internal Notes
    qDebug() << "Raw Link clicked:" << link;

    QString cleanLink = QUrl::fromPercentEncoding(link.toUtf8());

    if (cleanLink.startsWith("note://", Qt::CaseInsensitive)) {
        cleanLink = cleanLink.mid(7);
    }
    if (cleanLink.startsWith("[[")) {
        cleanLink = cleanLink.mid(2);
    }
    if (cleanLink.endsWith("]]")) {
        cleanLink = cleanLink.chopped(2);
    }

    qDebug() << "Processed Note Title:" << cleanLink;

    // Direct check if it matches an attachment filename
    if (m_repo) {
        QList<FileAttachment> atts;
        if (!m_activeProject.id.isNull()) atts.append(m_repo->getAttachmentsByProject(m_activeProject.id));
        if (!m_workspace.id.isNull()) atts.append(m_repo->getAttachmentsByWorkspace(m_workspace.id));
        for (const auto& a : atts) {
            if (a.fileName.compare(cleanLink, Qt::CaseInsensitive) == 0) {
                if (QFile::exists(a.relativePath)) {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(a.relativePath));
                    return;
                }
            }
        }
    }

    if (m_noteManager && !m_workspace.id.isNull()) {
        QList<Note> notes = m_noteManager->recentNotes(m_workspace.id);
        for (const auto& note : notes) {
            if (note.title.compare(cleanLink, Qt::CaseInsensitive) == 0) {
                loadNote(note.id.toString(QUuid::WithoutBraces));
                return;
            }
        }
    }

    qWarning() << "Could not find a note or attachment matching:" << cleanLink;
}

QString wsNotePage::attachFileToCurrentNote(const QString& localFilePath)
{
    if (!m_repo || localFilePath.isEmpty()) return "";

    // Clean up file:// prefix if dragged from QML/Qt DropArea
    QString cleanPath = localFilePath;
    if (cleanPath.startsWith("file:///", Qt::CaseInsensitive)) {
        cleanPath = cleanPath.mid(8);
    } else if (cleanPath.startsWith("file://", Qt::CaseInsensitive)) {
        cleanPath = cleanPath.mid(7);
    }
    cleanPath = QUrl::fromPercentEncoding(cleanPath.toUtf8());

    QFileInfo fi(cleanPath);
    if (!fi.exists() || !fi.isFile()) {
        qWarning() << "Cannot attach non-existent file:" << cleanPath;
        return "";
    }

    FileAttachment att;
    att.workspaceId = m_workspace.id;
    att.projectId = m_activeProject.id;
    if (att.projectId.isNull() && m_repo) {
        const auto projs = m_repo->getProjectsByWorkspace(m_workspace.id);
        if (!projs.isEmpty()) att.projectId = projs.first().id;
    }
    att.linkedEntityType = AttachmentEntityType::Note;
    if (m_currentNoteId != "-1" && !m_currentNoteId.isEmpty()) {
        att.linkedEntityId = QUuid::fromString(m_currentNoteId);
    }
    att.fileName = fi.fileName();
    att.relativePath = cleanPath;
    att.fileSize = fi.size();

    const QUuid newId = m_repo->createAttachment(att);
    if (newId.isNull()) {
        qWarning() << "Failed to store attachment in repository:" << fi.fileName();
        return "";
    }

    return QString("\n![[%1]]\n").arg(fi.fileName());
}

void wsNotePage::openAttachmentDialog()
{
    const QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Select Files to Attach"),
        QString(),
        tr("All Supported Files (*.png *.jpg *.jpeg *.gif *.svg *.pdf *.py *.cpp *.h *.js *.ts *.json *.csv *.tsv *.txt *.docx *.xlsx *.zip);;All Files (*.*)")
    );

    if (files.isEmpty()) return;

    QString tagsToInsert = "";
    for (const QString& file : files) {
        const QString tag = attachFileToCurrentNote(file);
        if (!tag.isEmpty()) {
            tagsToInsert += tag;
        }
    }

    if (!tagsToInsert.isEmpty()) {
        QString newContent = m_currentNoteContent;
        if (!newContent.isEmpty() && !newContent.endsWith("\n")) {
            newContent += "\n";
        }
        newContent += tagsToInsert;
        updateExistingNote(m_currentNoteId, newContent);
    }
}

QVariantList wsNotePage::searchAttachments(const QString& query)
{
    QVariantList results;
    if (!m_repo) return results;

    QList<FileAttachment> atts;
    if (!m_activeProject.id.isNull()) atts.append(m_repo->getAttachmentsByProject(m_activeProject.id));
    if (!m_workspace.id.isNull()) atts.append(m_repo->getAttachmentsByWorkspace(m_workspace.id));

    QSet<QString> seenNames;
    for (const auto& a : atts) {
        if (!seenNames.contains(a.fileName.toLower()) &&
            (query.isEmpty() || a.fileName.contains(query, Qt::CaseInsensitive))) {
            seenNames.insert(a.fileName.toLower());

            QVariantMap map;
            map["id"] = a.id.toString(QUuid::WithoutBraces);
            map["title"] = a.fileName;
            map["fileName"] = a.fileName;
            map["fileSize"] = NoteAttachmentRenderer::formatFileSize(a.fileSize);
            map["type"] = static_cast<int>(NoteAttachmentRenderer::detectFileType(a.fileName));
            results.append(map);
        }
    }
    return results;
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

QString wsNotePage::projectName() const
{
    if (!m_activeProject.name.isEmpty()) return m_activeProject.name;
    if (m_repo && !m_activeProject.id.isNull()) {
        const Project p = m_repo->getProjectById(m_activeProject.id);
        if (!p.name.isEmpty()) return p.name;
    }
    return tr("General");
}

QString wsNotePage::projectColor() const
{
    static const QStringList palette = {
        "#81C784", "#FFD700", "#9ACD32", "#20B2AA", "#FF69B4", "#64B5F6", "#BA68C8", "#FF8A65"
    };
    if (!m_activeProject.id.isNull()) {
        return palette.at(qAbs(qHash(m_activeProject.id.toString())) % palette.size());
    }
    return "#8B5CF6";
}

void wsNotePage::updateStats(const QString& content)
{
    m_characterCount = content.length();

    const QString trimmed = content.trimmed();
    if (trimmed.isEmpty()) {
        m_wordCount = 0;
    } else {
        m_wordCount = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    }

    m_readingTimeMinutes = qMax(1, (m_wordCount + 199) / 200);

    emit noteStatsChanged();
}
