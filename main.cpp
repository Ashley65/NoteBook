#include <QApplication>
#include <QDir>
#include <QLabel>
#include <QMouseEvent>
#include <QQuickStyle>

#include "UI/mainWIndow.h"



static QString getGlobalThemeStyle() {
    return R"(
        QWidget {
            background-color: #12141C;
            color: #E2E8F0;
            font-family: 'Segoe UI', system-ui, -apple-system, sans-serif;
            font-size: 13px;
        }

        QMainWindow {
            background-color: #12141C;
        }

        /* Modern Buttons */
        QPushButton {
            background: #1E202B;
            color: #F8FAFC;
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: 600;
        }

        QPushButton:hover {
            background: #282C3D;
            border-color: #6366F1;
            color: #FFFFFF;
        }

        QPushButton:pressed {
            background: #4F46E5;
        }

        /* Group Boxes & Cards */
        QGroupBox {
            background-color: #1A1C26;
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 12px;
            margin-top: 24px;
            font-weight: 700;
            font-size: 14px;
            color: #818CF8;
            padding: 16px;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 10px;
            color: #A5B4FC;
        }

        /* Input Fields */
        QLineEdit, QTextEdit, QPlainTextEdit, QComboBox {
            background-color: #1A1C26;
            color: #F8FAFC;
            border: 1px solid #2D3142;
            border-radius: 8px;
            padding: 8px 12px;
        }

        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
            border: 1px solid #6366F1;
            background-color: #212433;
        }

        /* Lists & Trees */
        QListWidget, QTreeWidget, QTableWidget {
            background-color: #1A1C26;
            border: 1px solid #2D3142;
            border-radius: 10px;
            outline: none;
            padding: 4px;
        }

        QListWidget::item, QTreeWidget::item {
            padding: 8px 12px;
            border-radius: 6px;
            color: #CBD5E1;
        }

        QListWidget::item:hover, QTreeWidget::item:hover {
            background-color: #252838;
            color: #FFFFFF;
        }

        QListWidget::item:selected, QTreeWidget::item:selected {
            background-color: #4338CA;
            color: #FFFFFF;
            font-weight: 600;
        }

        /* Menus */
        QMenu {
            background-color: #1A1C26;
            border: 1px solid #2D3142;
            border-radius: 8px;
            padding: 6px;
        }

        QMenu::item {
            padding: 8px 24px 8px 12px;
            border-radius: 6px;
            color: #E2E8F0;
        }

        QMenu::item:selected {
            background-color: #6366F1;
            color: #FFFFFF;
        }

        /* Scrollbars */
        QScrollBar:vertical {
            background: #12141C;
            width: 8px;
            margin: 0px;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical {
            background: #2D3142;
            min-height: 20px;
            border-radius: 4px;
        }

        QScrollBar::handle:vertical:hover {
            background: #6366F1;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )";
}

int main(int argc, char *argv[]) {
    QQuickStyle::setStyle("Basic");
    QApplication app(argc, argv);
    app.setOrganizationName("none");
    app.setApplicationName("flow");
    app.setStyleSheet(getGlobalThemeStyle());
    Q_INIT_RESOURCE(resources);
    qDebug() << "app=" << QCoreApplication::applicationFilePath();
    qDebug() << "rcc list :/icons=" << QDir(":/icons").entryList(QDir::Files);

    MainWindow window;

    window.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    window.show();
    return app.exec();
}