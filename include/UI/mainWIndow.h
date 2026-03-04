//
// Created by DevAccount on 21/11/2025.
//
#pragma once

#ifdef _WIN32
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#endif

#ifdef _WIN32
#include <QWindow>
#include <QScreen>
#include <QGuiApplication>
#endif


#ifndef TASKHELPER_MAINWINDOW_H
#define TASKHELPER_MAINWINDOW_H
#pragma once
#include <QFrame>
#include <QWidget>
#include "components/Top_Menubar/windowsAction.h"
#include "components/Top_Menubar/InfoBar.h"
#include "components/Top_Menubar/NavigationBar.h"
#include "components/Top_Menubar/MenuButtonBar.h"
#include "components/SIde_Bar/sideBar.h"
#include "helpers/AppStateController.h"
#include "Data/workspace/WorkspaceRepository.h"
#include "components/Content_Windows/MainContentView.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);


public slots:
    static void goBack();
    void goForward();
    void refreshPage();

private:
    AppStateController* m_stateController {nullptr};
    WorkspaceRepository* m_workspaceRepo {nullptr};
    MainContentView* m_mainContent {nullptr};

    static QFrame* createWidget(const QString& title,
                                const QString& color,
                                const QString& extraStyle = "",
                                bool withBorder = true,
                                bool transparent = false);
    WindowsActionsBar* m_windowActionsBar {nullptr};
    NavigationBar* m_navigationBar {nullptr};
    InfoBar* m_infoBar {nullptr};
    MenuButtonBar* m_menuButtonBar {nullptr};
    QPushButton* m_floatingToggleButton  {nullptr};
    void setupWindowActionsBar();
    void setupInfoBar();
    void setupNavigationBar();
    void updateFloatingToggleButtonVisibility();

    void setupSideBar();
    void setupSidebarConnections();

    void updateWindowTheme();
    [[nodiscard]] bool isDarkModeEnabled() const;

    enum Region {
        None       = 0,
        Left       = 1 << 0,
        Right      = 1 << 1,
        Top        = 1 << 2,
        Bottom     = 1 << 3,
        TopLeft    = Top | Left,
        TopRight   = Top | Right,
        BottomLeft = Bottom | Left,
        BottomRight= Bottom | Right
    };

    void setCursorForRegion(Region r);
    [[nodiscard]] Region hitTest(const QPoint& p) const;

    QFrame *topBarFrame = nullptr; // the area used for dragging (like a title bar)
    QFrame *sideBarFrame = nullptr; // the sidebar area
    SideBar* m_sideBar {nullptr};
    bool dragging = false;
    bool resizing = false;
    QPoint dragStartGlobal;
    QRect startGeom;
    Region currentRegion = None;
    const int borderWidth = 6;
    QRect lastNormalGeometry;
    bool haveNormalGeometry = {false};

    void applyWinVisualStyle();
    void ensureSnapFriendlyStyle() const;
    void rememberNormalGeometry();




protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent* e) override;
    void leaveEvent(QEvent *e) override;

    void showEvent(QShowEvent *e) override;

    void changeEvent(QEvent *e) override;

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif



};



#endif //TASKHELPER_MAINWINDOW_H
