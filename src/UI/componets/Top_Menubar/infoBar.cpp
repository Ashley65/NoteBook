//
// Created by DevAccount on 02/12/2025.
//

#include "UI/components/Top_Menubar/InfoBar.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QApplication>


InfoBar::InfoBar(QWidget* parent) : QWidget(parent)
{
    initUi();
    // Default app name to the top-level window title if available
    if (QWidget* win = window()) {
        m_appNameLabel->setText(win->windowTitle());
    }
}

void InfoBar::initUi()
{
    m_layout = new QHBoxLayout(this);

    // App name label
    m_appNameLabel = new QLabel("App Name", this);
    QFont appFont = m_appNameLabel->font();
    appFont.setBold(true);
    m_appNameLabel->setFont(appFont);

    // Separator
    QLabel *separator = new QLabel("|", this);
    separator->setStyleSheet("color: #888;");

    // Current screen label
    m_currentScreenLabel = new QLabel("Working File", this);

    // Add widgets to layout
    m_layout->addWidget(m_appNameLabel);
    m_layout->addWidget(separator);
    m_layout->addWidget(m_currentScreenLabel);
    m_layout->addStretch();

    // Set spacing and margins
    m_layout->setSpacing(8);
    m_layout->setContentsMargins(10, 5, 10, 5);

    // Style the info bar
    setStyleSheet("InfoBar { background-color: #3c4043; color: white; }");

    // Set a fixed or minimum height
    setMinimumHeight(35);
    setMaximumHeight(35);
}

void InfoBar::setAppName(const QString& name)
{
    m_appNameLabel->setText(name);
}

void InfoBar::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        emit maximiseRestoreRequested();
        e->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(e);
}

void InfoBar::setCurrentScreenLabel(const QString& name)
{
    m_currentScreenLabel->setText(name);
}



