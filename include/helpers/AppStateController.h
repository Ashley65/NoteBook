//
// Created by DevAccount on 23/01/2026.
//

#ifndef TASKHELPER_APPSTATECONTROLLER_H
#define TASKHELPER_APPSTATECONTROLLER_H
#pragma once
#include <QString>
#include <QObject>
#include <QUuid>


struct AppContext
{
    QUuid activeWorkspaceId;
    QString activeWorkspaceName;
    bool isCompact;
    bool isDarkMode;
    QUuid selectedTaskId;
    QString selectedFilter;
};


class AppStateController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString activeWorkspaceName READ activeWorkspaceName NOTIFY activeWorkspaceChanged)
    Q_PROPERTY(bool isCompact READ isCompact WRITE setCompact NOTIFY compactChanged)
    Q_PROPERTY(bool isDarkMode READ isDarkMode WRITE setDarkMode NOTIFY darkModeChanged)
    Q_PROPERTY(QUuid selectedTaskId READ selectedTaskId WRITE setSelectedTaskId NOTIFY selectedTaskChanged)
    Q_PROPERTY(QString selectedFilter READ selectedFilter WRITE setSelectedFilter NOTIFY selectedFilterChanged)

public:
    explicit AppStateController(QObject* parent = nullptr);
    [[nodiscard]] const AppContext& context() const;

    // Getters
    [[nodiscard]] QString activeWorkspaceName() const { return ctx_.activeWorkspaceName; }
    [[nodiscard]] bool isCompact() const { return ctx_.isCompact; }
    [[nodiscard]] bool isDarkMode() const { return ctx_.isDarkMode; }
    [[nodiscard]] QUuid selectedTaskId() const { return ctx_.selectedTaskId; }
    [[nodiscard]] QString selectedFilter() const { return ctx_.selectedFilter; }

public slots:
    void setActiveWorkspace(const QUuid& id, const QString& name);
    void setCompact(bool value);
    void setDarkMode(bool value);
    void setSelectedTaskId(const QUuid& taskId);
    void setSelectedFilter(const QString& filter);

signals:
    void activeWorkspaceChanged();
    void compactChanged();
    void darkModeChanged();
    void selectedTaskChanged();
    void selectedFilterChanged();

private:
    AppContext ctx_;
};
#endif //TASKHELPER_APPSTATECONTROLLER_H