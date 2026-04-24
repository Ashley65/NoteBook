import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    color: "transparent"
    radius: 10
    border.color: "#3B4048"
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        Text {
            text: "Recent Notes"
            color: "#FFFFFF"
            font.pixelSize: 20
            font.bold: true
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8
            clip: true
            model: typeof wsHomePage !== "undefined" ? wsHomePage.recentNotes : 0

            delegate: Rectangle {
                width: ListView.view.width
                height: 80
                color: "#2A2D35"
                radius: 8
                border.color: "#3B4048"
                border.width: 1

                Text {
                    text: modelData.title
                    color: "white"
                    anchors.margins: 12
                    anchors.top: parent.top
                    anchors.left: parent.left
                }
            }
        }


        Button {
            id: btnViewAllNotes
            text: "View All Notes >"
            onClicked: console.log("Mock: View All Notes Clicked")

            Layout.alignment: Qt.AlignHCenter


            contentItem: Text {
                text: btnViewAllNotes.text
                color: "#E2E8F0"
                font.pixelSize: 14
                font.weight: Font.Medium
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                implicitWidth: 130
                implicitHeight: 40
                color: btnViewAllNotes.hovered ? Qt.rgba(1, 1, 1, 0.15) : Qt.rgba(1, 1, 1, 0.05)
                radius: 8
                border.color: "#3B4048"
                border.width: 1

                Behavior on color { ColorAnimation { duration: 150 } }
            }
        }
    }
}
