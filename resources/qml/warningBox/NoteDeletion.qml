import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: "#1E293B"
    radius: 10
    border.color: "#3B4048"
    border.width: 1

    signal confirmDelete()
    signal cancel()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        Text {
            text: "Delete Note?"
            color: "#FFFFFF"
            font.pixelSize: 20
            font.bold: true
        }

        Text {
            text: "Are you sure you want to delete this note? This action cannot be undone."
            color: "#94A3B8"
            font.pixelSize: 14
            wrapMode: Text.Wrap
        }

        RowLayout {
            spacing: 12
            Layout.alignment: Qt.AlignRight

            Button {
                text: "Cancel"
                background: Rectangle {
                    color: "transparent"
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.hovered ? "#FFFFFF" : "#94A3B8" // subtle hover effect
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.cancel()
            }

            Button {
                text: "Delete"
                background: Rectangle {
                    color: "#EF4444"
                    radius: 4
                }
                onClicked: root.confirmDelete()
            }
        }
    }
}