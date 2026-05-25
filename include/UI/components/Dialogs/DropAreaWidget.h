//
// Created by DevAccount on 16/05/2026.
//

#ifndef TASKHELPER_DROPAREAWIDGET_H
#define TASKHELPER_DROPAREAWIDGET_H
#pragma once
#include <QWidget>

#include <QWidget>
#include <QStringList>
#include <QTimer>

class QDragEnterEvent;
class QDragLeaveEvent;
class QDropEvent;
class QPaintEvent;

class DropAreaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DropAreaWidget(QWidget* parent = nullptr);


public slots:
    void resetToIdleState();
    void setAllowedExtensions(const QStringList& extensions);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void paintEvent(QPaintEvent* event) override;


signals:
    void filesDropped(const QStringList& filePaths);
    void browseRequested();

private slots:
    void onFlashTimerTimeout();


private:
    bool m_isDragActive = false;
    bool m_flashState = false;
    QTimer* m_flashTimer = nullptr;
    QStringList m_allowedExtensions;



};

#endif //TASKHELPER_DROPAREAWIDGET_H
