import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "wsProjectPage" as WsProj

Rectangle {
    id: root
    anchors.fill: parent
    color: "#14151F"

    readonly property bool isStacked: root.width < 740

    Flickable {
        id: scrollArea
        anchors.fill: parent
        contentWidth: width
        contentHeight: contentCol.implicitHeight + 48
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        Item {
            width: scrollArea.width
            height: Math.max(scrollArea.height, contentCol.implicitHeight + 48)

            ColumnLayout {
                id: contentCol
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: root.width < 640 ? 16 : 24
                spacing: 18

                // 1. Project Header & Actions
                WsProj.ProjectHeader {
                    Layout.fillWidth: true
                }

                // 2. Project Health & Progress Velocity Section
                WsProj.ProjectProgressSection {
                    Layout.fillWidth: true
                }

                // 3. Tasks & Documentation Columns
                GridLayout {
                    id: columnsLayout
                    Layout.fillWidth: true
                    columns: root.isStacked ? 1 : 2
                    rowSpacing: 16
                    columnSpacing: 16

                    WsProj.ProjectTasksColumn {
                        Layout.fillWidth: true
                        Layout.preferredHeight: root.isStacked ? 400 : 500
                        Layout.minimumHeight: 320
                    }

                    WsProj.ProjectNotesFilesColumn {
                        Layout.fillWidth: true
                        Layout.preferredHeight: root.isStacked ? 480 : 500
                        Layout.minimumHeight: 360
                    }
                }
            }
        }
    }
}
