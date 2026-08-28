import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: columnRoot
    property string title: "Column"
    property color statusColor: "#3B82F6"
    property int count: 0
    property var taskModel: []
    property int targetStatus: 0

    color: "#111520"
    radius: 12
    border.color: "#1F2637"
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        // Column Header Row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            // Status Color Dot
            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: columnRoot.statusColor
            }

            // Column Title
            Text {
                text: columnRoot.title
                color: "#FFFFFF"
                font.pixelSize: 14
                font.bold: true
                Layout.alignment: Qt.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
            }

            // Count Badge
            Rectangle {
                implicitWidth: countText.implicitWidth + 14
                implicitHeight: 20
                radius: 10
                color: "#1E2538"

                Text {
                    id: countText
                    anchors.centerIn: parent
                    text: columnRoot.count.toString()
                    color: "#94A3B8"
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                }
            }
        }

        // Tasks ListView
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
            clip: true
            model: columnRoot.taskModel

            delegate: TaskCard {}

            // Empty state placeholder
            Text {
                anchors.centerIn: parent
                visible: listView.count === 0
                text: "No tasks"
                color: "#475569"
                font.pixelSize: 13
            }
        }
    }
}
