//
// Created by DevAccount on 18/12/2025.
//

#ifndef TASKHELPER_FOOTERSECTION_H
#define TASKHELPER_FOOTERSECTION_H
#pragma once

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QToolButton>

#include "UI/components/SIde_Bar/sideBar.h"


class FooterSection : public QWidget
{
    Q_OBJECT
public:
    explicit FooterSection(QWidget* parent = nullptr);

    // Call this when the sidebar mode changes so the button icon can update
    void setMode(SideBar::Mode mode);

    signals:
        // Signal to tell the parent (SideBar) to switch modes
        void modeCycleRequested();
        // An Signal to inform sync status changes (not implemented here)
        // void syncStatusChanged(const QString& status);

protected:
    void changeEvent(QEvent *event) override;

private:
    QToolButton* modeBtn;

    QLabel* syncStatus;
    QLabel* syncDot;
    SideBar::Mode m_currentMode { SideBar::Mode::Default };
};
#endif //TASKHELPER_FOOTERSECTION_H