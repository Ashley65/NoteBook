//
// Created by DevAccount on 23/01/2026.
//

#ifndef TASKHELPER_APPSTATECONTROLLER_H
#define TASKHELPER_APPSTATECONTROLLER_H
#pragma once
#include <QString>
#include <QObject>


struct AppContext
{
    QString activeWorkspaceId;
    QString activeWorkspaceName;
};


class AppStateController : public QObject
{
    Q_OBJECT
public:
    explicit AppStateController(QObject* parent = nullptr);
    [[nodiscard]] const AppContext& context() const;

public slots:
    void setActiveWorkspace(const QString& id, const QString& name);


signals:
    void contextChanged(const AppContext& ctx);

private:
    AppContext ctx_;
};
#endif //TASKHELPER_APPSTATECONTROLLER_H