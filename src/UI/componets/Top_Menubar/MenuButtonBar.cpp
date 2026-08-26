//
// Created by DevAccount on 14/12/2025.
//


#include "UI/components/Top_Menubar/MenuButtonBar.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QMenu>
#include <QIcon>

MenuButtonBar::MenuButtonBar(QWidget *parent) : QWidget(parent)
{
    initUi();
}

void MenuButtonBar::setButtonStyleSheet(const QString& css) const
{
    if (m_menuBtn)
    {
        m_menuBtn->setStyleSheet(css);
    }
}

void MenuButtonBar::initUi()
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    constexpr int buttonSize = 28;
    constexpr QSize iconSize(16, 16);

    auto loadIcon = [](const QString &name) -> QIcon {
        QIcon ic = QIcon::fromTheme(name);
        if (!ic.isNull())
            return ic;
        return QIcon(QStringLiteral(":/icons/") + name + QStringLiteral(".svg"));
    };

    m_menuBtn = new QPushButton(this);
    m_menuBtn->setToolTip("Settings");
    m_menuBtn->setAccessibleName("Settings");
    m_menuBtn->setFlat(true);
    m_menuBtn->setFixedSize(buttonSize, buttonSize);
    m_menuBtn->setIcon(loadIcon(QStringLiteral("settings")));
    m_menuBtn->setIconSize(iconSize);
    m_menuBtn->setStyleSheet(
        "QPushButton { padding: 0px; border: none; border-radius: 4px; }"
        "QPushButton::menu-indicator { image: none; width: 0px; margin: 0px; padding: 0px; }"
    );

    layout->addWidget(m_menuBtn);

    auto *menu = new QMenu(this);
    menu->setStyleSheet(
        "QMenu {"
        "  background-color: #1E202B;"
        "  color: #FFFFFF;"
        "  border: 1px solid rgba(255, 255, 255, 0.1);"
        "  border-radius: 6px;"
        "  padding: 4px;"
        "}"
        "QMenu::item {"
        "  padding: 6px 20px 6px 10px;"
        "  border-radius: 4px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: #2D3042;"
        "}"
        "QMenu::separator {"
        "  height: 1px;"
        "  background-color: rgba(255, 255, 255, 0.08);"
        "  margin: 4px 0px;"
        "}"
    );

    auto *settingsAct = menu->addAction("Settings");
    auto *aboutAct    = menu->addAction("About");
    menu->addSeparator();
    auto *exitAct     = menu->addAction("Exit");

    connect(settingsAct, &QAction::triggered, this, &MenuButtonBar::settingRequested);
    connect(aboutAct,    &QAction::triggered, this, &MenuButtonBar::aboutRequested);
    connect(exitAct,     &QAction::triggered, this, &MenuButtonBar::exitRequested);

    m_menuBtn->setMenu(menu);
}

QMenu *MenuButtonBar::menuBar() const
{
    return m_menuBtn ? m_menuBtn->menu() : nullptr;
}
