//
// Created by DevAccount on 23/12/2025.
//

#include <UI/components/SIde_Bar/componets/PrimaryActionSection.h>
#include <QVBoxLayout>
#include <QEvent>
#include <QToolButton>
#include <QMenu>
#include <QDynamicPropertyChangeEvent>

WorkspaceContextSection::WorkspaceContextSection(QWidget *parent) : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    // Margins: Top/Bottom 10px, Left/Right 5px
    layout->setContentsMargins(5, 10, 5, 10);
    layout->setSpacing(0);

    mainBtn = new QToolButton(this); // Changed from QPushButton
    mainBtn->setObjectName("PrimaryBtn");
    mainBtn->setCursor(Qt::PointingHandCursor);


    mainBtn->setIcon(QIcon(":/icons/dashboard.svg"));
    mainBtn->setIconSize(QSize(20, 20));

    layout->addWidget(mainBtn);

    // Initial State: Default (Not Compact)
    updateStyle(false);

    connect(mainBtn, &QToolButton::clicked, this, &WorkspaceContextSection::triggered); // Changed from QPushButton


    mainBtn->setPopupMode(QToolButton::InstantPopup);

    // Create the dropdown menu
    QMenu* workspaceMenu = new QMenu(mainBtn);
    auto* header = new QAction("Workspace: Personal", workspaceMenu);
    header->setEnabled(false);

    auto* switchAction   = new QAction("Switch Workspace…", workspaceMenu);
    auto* settingsAction = new QAction("Workspace Settings…", workspaceMenu);
    auto* addAction      = new QAction("+ Add New Workspace", workspaceMenu);

    workspaceMenu->addAction(header);
    workspaceMenu->addSeparator();
    workspaceMenu->addAction(switchAction);
    workspaceMenu->addAction(settingsAction);
    workspaceMenu->addSeparator();
    workspaceMenu->addAction(addAction);

    switchAction->setData("Switch");
    settingsAction->setData("workspaceSettings");
    addAction->setData("addWorkspace");


    workspaceMenu->setStyleSheet(R"(
        /* Remove default margin/padding */
        QMenu {
            background-color: #2B3038;
            border: 1px solid #444C58;
        }
        /* Style each action */
        QMenu::item {
            padding: 6px 12px;
            color: #D0D5DD;
            font-size: 13px;
        }
        /* Highlight on hover */
        QMenu::item:selected {
            background-color: #3C4655;
            color: #FFFFFF;
        }
        /* Optional: give a subtle icon to actions */
        QMenu::item:selected:has-children {
            background-color: #3C4655;
        }
    )");

    mainBtn->setMenu(workspaceMenu);

    connect(workspaceMenu, &QMenu::triggered, this, &WorkspaceContextSection::onMenuActionTriggered);
    connect(mainBtn, &QToolButton::clicked, this, &WorkspaceContextSection::triggered);
}

void WorkspaceContextSection::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::DynamicPropertyChange) {
        auto* propEvent = dynamic_cast<QDynamicPropertyChangeEvent*>(event);
        if (propEvent->propertyName() == "compact") {
            bool isCompact = property("compact").toBool();
            updateStyle(isCompact);
        }
    }
    QWidget::changeEvent(event);
}

void WorkspaceContextSection::onMenuActionTriggered(QAction *action) {
    const QString cmd = action->data().toString();

    if (cmd == "Switch") {
        emit requestWorkspaceSwitch();
    } else if (cmd == "workspaceSettings") {
        emit requestWorkspaceSettings(currentWorkspaceID_);
    } else if (cmd == "addWorkspace") {
        emit requestWorkspaceCreate();
    }

}

void WorkspaceContextSection::updateStyle(bool compact)
{
    if (compact) {
        // --- COMPACT MODE ---
        mainBtn->setText(""); // Hide Text
        mainBtn->setToolTip("Personal Workspace");
        mainBtn->setToolButtonStyle(Qt::ToolButtonIconOnly); // Centre the icon

        // Centre the icon, make it look like a square button
        mainBtn->setStyleSheet(R"(
            QPushButton#PrimaryBtn {
                background-color: transparent;
                border: 1px solid transparent;
                border-radius: 6px;
                padding: 6px;
            }
            QPushButton#PrimaryBtn:hover {
                background-color: #1A1D2D;
                border: 1px solid #333646;
            }
        )");
    } else {
        // --- DEFAULT MODE ---
        mainBtn->setText(" " + currentWorkspaceName_ + " ▼");
        mainBtn->setToolTip("");
        mainBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); // Reset to default

        // Left-align text, add padding for the "Wide" look
        mainBtn->setStyleSheet(R"(
            QPushButton#PrimaryBtn {
                background-color: transparent;
                color: #E0E0E0;
                font-weight: bold;
                font-size: 14px;
                border: 1px solid transparent;
                border-radius: 6px;
                padding: 8px 12px;
                text-align: left; /* Left align text */
            }
            QPushButton#PrimaryBtn:hover {
                background-color: #1A1D2D;
                border: 1px solid #333646;
            }
        )");
    }
    // TOD


}
