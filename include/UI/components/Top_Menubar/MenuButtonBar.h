//
// Created by DevAccount on 14/12/2025.
//

#ifndef TASKHELPER_MENUBUTTONBAR_H
#define TASKHELPER_MENUBUTTONBAR_H
#pragma once
#include <QWidget>

class QPushButton;
class Menubar;

class MenuButtonBar : public QWidget
{
    Q_OBJECT

private:
    void initUi();
    QPushButton *m_menuBtn{nullptr};



signals:
    void settingRequested();
    void aboutRequested();
    void exitRequested();

public:
    explicit MenuButtonBar(QWidget *parent = nullptr);
    void setButtonStyleSheet(const QString &css) const;
    QMenu *menuBar() const;
};



#endif //TASKHELPER_MENUBUTTONBAR_H