#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QMouseEvent>
#include <QQuickStyle>

#include "UI/mainWIndow.h"



int main(int argc, char *argv[]) {
    QQuickStyle::setStyle("Basic");
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(resources);
    qDebug() << "app=" << QCoreApplication::applicationFilePath();
    qDebug() << "rcc list :/icons=" << QDir(":/icons").entryList(QDir::Files);

    MainWindow window;
    app.setOrganizationName("none");
    app.setApplicationName("flow");

    window.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    window.show();
    return app.exec();
}