#include <QTest>
#include <QSignalSpy>
#include "Data/workspace/Manager/TabManager.h"

class TabManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void testInitialState();
    void testNavigationStack();
    void testBackAndForward();
    void testForwardTruncationOnNewNavigation();
    void testMultiTabHistoryIsolation();
    void testSameStateNoDuplicate();
};

void TabManagerTest::testInitialState()
{
    TabManager manager;
    QCOMPARE(manager.canGoBack(), false);
    QCOMPARE(manager.canGoForward(), false);

    QUuid wsId = QUuid::createUuid();
    manager.addTab("Home", "Home", wsId, "#3B82F6");

    QCOMPARE(manager.canGoBack(), false);
    QCOMPARE(manager.canGoForward(), false);
    QCOMPARE(manager.activeTabId(), wsId);
}

void TabManagerTest::testNavigationStack()
{
    TabManager manager;
    QUuid homeId = QUuid::createUuid();
    QUuid projId = QUuid::createUuid();
    QUuid noteId = QUuid::createUuid();

    manager.addTab("Home", "Home", homeId, "#3B82F6");

    QSignalSpy historySpy(&manager, &TabManager::navigationHistoryChanged);
    QVERIFY(historySpy.isValid());

    // Navigate to Project
    manager.navigateActiveTab("Project A", "Project", projId, "#81C784");
    QCOMPARE(manager.activeTabId(), projId);
    QCOMPARE(manager.canGoBack(), true);
    QCOMPARE(manager.canGoForward(), false);
    QCOMPARE(historySpy.count(), 1);
    QCOMPARE(historySpy.last().at(0).toBool(), true);  // canGoBack
    QCOMPARE(historySpy.last().at(1).toBool(), false); // canGoForward

    // Navigate to Note
    manager.navigateActiveTab("Note 1", "Note", noteId, "#C586C0");
    QCOMPARE(manager.activeTabId(), noteId);
    QCOMPARE(manager.canGoBack(), true);
    QCOMPARE(manager.canGoForward(), false);
}

void TabManagerTest::testBackAndForward()
{
    TabManager manager;
    QUuid homeId = QUuid::createUuid();
    QUuid projId = QUuid::createUuid();
    QUuid noteId = QUuid::createUuid();

    manager.addTab("Home", "Home", homeId, "#3B82F6");
    manager.navigateActiveTab("Project A", "Project", projId, "#81C784");
    manager.navigateActiveTab("Note 1", "Note", noteId, "#C586C0");

    QSignalSpy openSpy(&manager, &TabManager::tabOpened);

    // Step 1: Back to Project
    manager.goBack();
    QCOMPARE(manager.activeTabId(), projId);
    QCOMPARE(manager.canGoBack(), true);
    QCOMPARE(manager.canGoForward(), true);
    QCOMPARE(openSpy.last().at(0).toString(), QString("Project"));
    QCOMPARE(openSpy.last().at(1).toUuid(), projId);

    // Step 2: Back to Home
    manager.goBack();
    QCOMPARE(manager.activeTabId(), homeId);
    QCOMPARE(manager.canGoBack(), false);
    QCOMPARE(manager.canGoForward(), true);
    QCOMPARE(openSpy.last().at(0).toString(), QString("Home"));
    QCOMPARE(openSpy.last().at(1).toUuid(), homeId);

    // Step 3: Forward to Project
    manager.goForward();
    QCOMPARE(manager.activeTabId(), projId);
    QCOMPARE(manager.canGoBack(), true);
    QCOMPARE(manager.canGoForward(), true);
    QCOMPARE(openSpy.last().at(0).toString(), QString("Project"));

    // Step 4: Forward to Note
    manager.goForward();
    QCOMPARE(manager.activeTabId(), noteId);
    QCOMPARE(manager.canGoBack(), true);
    QCOMPARE(manager.canGoForward(), false);
    QCOMPARE(openSpy.last().at(0).toString(), QString("Note"));
}

void TabManagerTest::testForwardTruncationOnNewNavigation()
{
    TabManager manager;
    QUuid id1 = QUuid::createUuid();
    QUuid id2 = QUuid::createUuid();
    QUuid id3 = QUuid::createUuid();
    QUuid id4 = QUuid::createUuid();

    manager.addTab("Page 1", "Home", id1, "#3B82F6");
    manager.navigateActiveTab("Page 2", "Project", id2, "#81C784");
    manager.navigateActiveTab("Page 3", "Note", id3, "#C586C0");

    // Go back to Page 2
    manager.goBack();
    QCOMPARE(manager.activeTabId(), id2);
    QCOMPARE(manager.canGoForward(), true);

    // Navigate to Page 4: Page 3 should be dropped from forward history
    manager.navigateActiveTab("Page 4", "TaskBoard", id4, "#6366F1");
    QCOMPARE(manager.activeTabId(), id4);
    QCOMPARE(manager.canGoBack(), true);
    QCOMPARE(manager.canGoForward(), false);

    // Go back: should be Page 2
    manager.goBack();
    QCOMPARE(manager.activeTabId(), id2);
    QCOMPARE(manager.canGoBack(), true);
    QCOMPARE(manager.canGoForward(), true);

    // Go back: should be Page 1
    manager.goBack();
    QCOMPARE(manager.activeTabId(), id1);
    QCOMPARE(manager.canGoBack(), false);
    QCOMPARE(manager.canGoForward(), true);
}

void TabManagerTest::testMultiTabHistoryIsolation()
{
    TabManager manager;
    QUuid tab1Id1 = QUuid::createUuid();
    QUuid tab1Id2 = QUuid::createUuid();
    QUuid tab2Id = QUuid::createUuid();

    // Tab 1 setup
    manager.addTab("Tab 1 Home", "Home", tab1Id1, "#3B82F6");
    manager.navigateActiveTab("Tab 1 Sub", "Project", tab1Id2, "#81C784");
    QCOMPARE(manager.canGoBack(), true);
    QCOMPARE(manager.canGoForward(), false);

    // Add Tab 2
    manager.addTab("Tab 2 Home", "Home", tab2Id, "#EF4444");
    // Tab 2 is active and has no back history
    QCOMPARE(manager.activeTabId(), tab2Id);
    QCOMPARE(manager.canGoBack(), false);
    QCOMPARE(manager.canGoForward(), false);

    // Switch back to Tab 1
    manager.setActiveTabId(tab1Id2);
    QCOMPARE(manager.activeTabId(), tab1Id2);
    QCOMPARE(manager.canGoBack(), true);
    QCOMPARE(manager.canGoForward(), false);

    // Switch back to Tab 2
    manager.setActiveTabId(tab2Id);
    QCOMPARE(manager.activeTabId(), tab2Id);
    QCOMPARE(manager.canGoBack(), false);
    QCOMPARE(manager.canGoForward(), false);
}

void TabManagerTest::testSameStateNoDuplicate()
{
    TabManager manager;
    QUuid id1 = QUuid::createUuid();

    manager.addTab("Home", "Home", id1, "#3B82F6");
    QCOMPARE(manager.canGoBack(), false);

    // Navigate to exact same view and context
    manager.navigateActiveTab("Home Updated", "Home", id1, "#3B82F6");
    QCOMPARE(manager.canGoBack(), false);
    QCOMPARE(manager.canGoForward(), false);
}

QTEST_MAIN(TabManagerTest)
#include "TabManager_test.moc"
