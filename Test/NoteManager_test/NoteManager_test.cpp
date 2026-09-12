#include <QTest>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QTemporaryDir>
#include "Data/workspace/WorkspaceRepository.h"
#include "Data/workspace/Manager/NoteManager.h"

class NoteManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testCreateAndFetchNote();
    void testRecentAndPinnedNotes();
    void testSetPinnedAndArchived();
    void testUpdateNote();
    void testDeleteNote();

private:
    QTemporaryDir* m_tempDir = nullptr;
    WorkspaceRepository* m_repo = nullptr;
    NoteManager* m_noteManager = nullptr;
    QUuid m_workspaceId;
    QUuid m_projectId;
};

void NoteManagerTest::initTestCase()
{
    Q_INIT_RESOURCE(resources);
    QStandardPaths::setTestModeEnabled(true);

    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());

    m_repo = new WorkspaceRepository();
    m_noteManager = new NoteManager(m_repo);

    m_workspaceId = m_repo->createWorkspace("Note Manager WS");
    Project proj;
    proj.workspaceId = m_workspaceId;
    proj.name = "Docs Project";
    m_projectId = m_repo->createProject(proj);
}

void NoteManagerTest::cleanupTestCase()
{
    delete m_noteManager;
    m_noteManager = nullptr;

    delete m_repo;
    m_repo = nullptr;

    delete m_tempDir;
    m_tempDir = nullptr;
}

void NoteManagerTest::testCreateAndFetchNote()
{
    NoteCreateRequest req;
    req.workspaceId = m_workspaceId;
    req.projectId = m_projectId;
    req.title = "Architecture Overview";
    req.content = "# System Architecture\nModular desktop app with Qt and SQLite.";
    req.isPinned = false;

    QUuid noteId = m_noteManager->createNote(req);
    QVERIFY(!noteId.isNull());

    Note fetched = m_noteManager->getNoteById(noteId);
    QCOMPARE(fetched.id, noteId);
    QCOMPARE(fetched.title, QString("Architecture Overview"));
    QCOMPARE(fetched.content, QString("# System Architecture\nModular desktop app with Qt and SQLite."));
    QCOMPARE(fetched.isPinned, false);

    auto all = m_noteManager->allNotes(m_workspaceId);
    bool found = false;
    for (const auto& n : all) {
        if (n.id == noteId) found = true;
    }
    QVERIFY(found);
}

void NoteManagerTest::testRecentAndPinnedNotes()
{
    // Create pinned note
    NoteCreateRequest pinnedReq;
    pinnedReq.workspaceId = m_workspaceId;
    pinnedReq.projectId = m_projectId;
    pinnedReq.title = "Important Guidelines";
    pinnedReq.content = "Read before committing code.";
    pinnedReq.isPinned = true;

    QUuid pinnedId = m_noteManager->createNote(pinnedReq);
    QVERIFY(!pinnedId.isNull());

    // Create unpinned note
    NoteCreateRequest unpinnedReq;
    unpinnedReq.workspaceId = m_workspaceId;
    unpinnedReq.projectId = m_projectId;
    unpinnedReq.title = "Quick Draft";
    unpinnedReq.content = "Temporary note.";
    unpinnedReq.isPinned = false;

    QUuid unpinnedId = m_noteManager->createNote(unpinnedReq);
    QVERIFY(!unpinnedId.isNull());

    // Verify pinnedNotes filter
    auto pinnedList = m_noteManager->pinnedNotes(m_workspaceId);
    bool foundPinned = false;
    bool foundUnpinnedInPinned = false;
    for (const auto& n : pinnedList) {
        if (n.id == pinnedId) foundPinned = true;
        if (n.id == unpinnedId) foundUnpinnedInPinned = true;
    }
    QVERIFY(foundPinned);
    QVERIFY(!foundUnpinnedInPinned);

    // Verify recentNotes contains both
    auto recentList = m_noteManager->recentNotes(m_workspaceId, 10);
    bool foundRecent1 = false;
    bool foundRecent2 = false;
    for (const auto& n : recentList) {
        if (n.id == pinnedId) foundRecent1 = true;
        if (n.id == unpinnedId) foundRecent2 = true;
    }
    QVERIFY(foundRecent1);
    QVERIFY(foundRecent2);
}

void NoteManagerTest::testSetPinnedAndArchived()
{
    NoteCreateRequest req;
    req.workspaceId = m_workspaceId;
    req.projectId = m_projectId;
    req.title = "Toggle Target Note";
    req.content = "Content to toggle.";
    req.isPinned = false;

    QUuid noteId = m_noteManager->createNote(req);
    QVERIFY(!noteId.isNull());

    // Toggle Pin
    m_noteManager->setPinned(noteId, true);
    Note pinned = m_noteManager->getNoteById(noteId);
    QCOMPARE(pinned.isPinned, true);

    m_noteManager->setPinned(noteId, false);
    Note unpinned = m_noteManager->getNoteById(noteId);
    QCOMPARE(unpinned.isPinned, false);

    // Toggle Archive
    m_noteManager->setArchived(noteId, true);
    Note archived = m_noteManager->getNoteById(noteId);
    QCOMPARE(archived.isArchived, true);

    // Archived notes should not appear in recentNotes or pinnedNotes
    auto recent = m_noteManager->recentNotes(m_workspaceId, 10);
    for (const auto& n : recent) {
        QVERIFY(n.id != noteId);
    }
}

void NoteManagerTest::testUpdateNote()
{
    NoteCreateRequest req;
    req.workspaceId = m_workspaceId;
    req.projectId = m_projectId;
    req.title = "Pre-Update Note";
    req.content = "Initial content.";
    QUuid noteId = m_noteManager->createNote(req);
    QVERIFY(!noteId.isNull());

    Note note = m_noteManager->getNoteById(noteId);
    note.title = "Post-Update Note";
    note.content = "Updated content.";
    m_noteManager->updateNote(note);

    Note updated = m_noteManager->getNoteById(noteId);
    QCOMPARE(updated.title, QString("Post-Update Note"));
    QCOMPARE(updated.content, QString("Updated content."));
}

void NoteManagerTest::testDeleteNote()
{
    NoteCreateRequest req;
    req.workspaceId = m_workspaceId;
    req.projectId = m_projectId;
    req.title = "Note To Delete";
    QUuid noteId = m_noteManager->createNote(req);
    QVERIFY(!noteId.isNull());

    m_noteManager->deleteNote(noteId);
    Note fetched = m_noteManager->getNoteById(noteId);
    QVERIFY(fetched.id.isNull());
}

QTEST_MAIN(NoteManagerTest)
#include "NoteManager_test.moc"
