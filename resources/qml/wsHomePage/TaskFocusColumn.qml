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
            text: "Task Focus"
            color: "#FFFFFF"
            font.pixelSize: 20
            font.bold: true
        }

        // Today tasks section
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 170
            color: "transparent"
            radius: 8
            border.color: "#3B4048"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                Text {
                    text: "Today"
                    color: "#FFFFFF"
                    font.pixelSize: 16
                    font.bold: true
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8
                    clip: true
                    model: typeof wsHomePage !== "undefined" ? wsHomePage.todayTasks : []

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 60
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
            }
        }

        // Use overdueTasks until upcomingTasks exists in C++
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            radius: 8
            border.color: "#3B4048"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                Text {
                    text: "Overdue"
                    color: "#FFFFFF"
                    font.pixelSize: 16
                    font.bold: true
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 10
                    clip: true
                    model: typeof wsHomePage !== "undefined" ? wsHomePage.overdueTasks : []

                    // Helper for "Due Apr 22" formatting
                    function dueLabel(value) {
                        if (!value) return ""
                        return "Due " + Qt.formatDateTime(value, "MMM d")
                    }

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 96
                        radius: 14
                        color: "#5A2525"          // card fill
                        border.color: "#7A3131"    // subtle red border
                        border.width: 1

                        // Left accent stripe like screenshot
                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: 4
                            radius: 2
                            color: "#FF6B57"
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12

                            // Icon area
                            Text {
                                text: "\u23F0" // alarm clock icon
                                color: "#FF8A78"
                                font.pixelSize: 20
                                Layout.alignment: Qt.AlignTop
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                RowLayout {
                                    Layout.fillWidth: true

                                    Text {
                                        text: modelData.title || "Untitled Task"
                                        color: "#FFE9E6"
                                        font.pixelSize: 18
                                        font.bold: true
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }

                                    Text {
                                        text: ListView.view.dueLabel(modelData.dueDate)
                                        color: "#F2C9C3"
                                        font.pixelSize: 14
                                    }
                                }

                                Text {
                                    text: ListView.view.dueLabel(modelData.dueDate)
                                    color: "#E7B8B1"
                                    font.pixelSize: 12
                                    visible: text.length > 4
                                }

                                Text {
                                    text: modelData.description || ""
                                    color: "#F6DBD8"
                                    font.pixelSize: 12
                                    elide: Text.ElideRight
                                    maximumLineCount: 1
                                    visible: text.length > 0
                                }
                            }
                        }
                    }
                }
            }
        }


        Button {
            id: btnViewAllTasks
            text: "View All Tasks >"
            onClicked: console.log("Mock: View All Tasks Clicked")

            Layout.alignment: Qt.AlignHCenter


            contentItem: Text {
                text: btnViewAllTasks.text
                color: "#E2E8F0"
                font.pixelSize: 14
                font.weight: Font.Medium
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                implicitWidth: 130
                implicitHeight: 40
                color: btnViewAllTasks.hovered ? Qt.rgba(1, 1, 1, 0.15) : Qt.rgba(1, 1, 1, 0.05)
                radius: 8
                border.color: "#3B4048"
                border.width: 1

                Behavior on color { ColorAnimation { duration: 150 } }
            }
        }
    }
}
