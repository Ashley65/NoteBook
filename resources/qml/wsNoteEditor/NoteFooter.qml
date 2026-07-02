import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    height: 30
    color: root.bgDarker
    border.color: root.borderDark
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.margins: 5

        Item { Layout.fillWidth: true } // Spacer

        Label {
            text: "Markdown Supported"
            color: root.textMuted
            font.pixelSize: 11
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: 10
        }
    }
}