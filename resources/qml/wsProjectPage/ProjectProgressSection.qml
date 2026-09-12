import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    Layout.fillWidth: true
    implicitHeight: mainLayout.implicitHeight + 28
    radius: 12
    color: "#181A26"
    border.color: Qt.rgba(1, 1, 1, 0.08)
    border.width: 1

    readonly property int totalTasks: typeof wsProjectPage !== "undefined" ? wsProjectPage.totalTasksCount : 0
    readonly property int completedTasks: typeof wsProjectPage !== "undefined" ? wsProjectPage.completedTasksCount : 0
    readonly property int inProgressTasks: typeof wsProjectPage !== "undefined" ? wsProjectPage.inProgressTasksCount : 0
    readonly property int pendingTasks: typeof wsProjectPage !== "undefined" ? wsProjectPage.pendingTasksCount : 0
    readonly property int completionPct: typeof wsProjectPage !== "undefined" ? wsProjectPage.completionPercentage : 0

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        Text {
            text: "Project Health & Progress"
            color: "#FFFFFF"
            font.pixelSize: 15
            font.bold: true
            font.family: "Inter"
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 18

            // --- Circular Completion Donut / Gauge ---
            Rectangle {
                width: 76
                height: 76
                radius: 38
                color: Qt.rgba(0.06, 0.72, 0.51, 0.12)
                border.color: root.completionPct >= 75 ? "#10B981" : (root.completionPct >= 40 ? "#F59E0B" : "#3B82F6")
                border.width: 3

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 1

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: root.completionPct + "%"
                        color: "#FFFFFF"
                        font.pixelSize: 17
                        font.bold: true
                        font.family: "Inter"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "Done"
                        color: "#94A3B8"
                        font.pixelSize: 10
                        font.family: "Inter"
                    }
                }
            }

            // --- 3 Stacked Progress Breakdown Bars ---
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                // 1. Completed Bar
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Completed (" + root.completedTasks + " tasks)"
                            color: "#E2E8F0"
                            font.pixelSize: 11
                            font.weight: Font.Medium
                            font.family: "Inter"
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: (root.totalTasks > 0 ? Math.round((root.completedTasks * 100) / root.totalTasks) : 0) + "%"
                            color: "#10B981"
                            font.pixelSize: 11
                            font.bold: true
                            font.family: "Inter"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 7
                        radius: 3.5
                        color: Qt.rgba(1, 1, 1, 0.08)

                        Rectangle {
                            height: parent.height
                            width: root.totalTasks > 0 ? (parent.width * (root.completedTasks / root.totalTasks)) : 0
                            radius: 3.5
                            color: "#10B981"
                            Behavior on width { NumberAnimation { duration: 250; easing.type: Easing.OutQuad } }
                        }
                    }
                }

                // 2. In Progress Bar
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "In Progress (" + root.inProgressTasks + " tasks)"
                            color: "#E2E8F0"
                            font.pixelSize: 11
                            font.weight: Font.Medium
                            font.family: "Inter"
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: (root.totalTasks > 0 ? Math.round((root.inProgressTasks * 100) / root.totalTasks) : 0) + "%"
                            color: "#F59E0B"
                            font.pixelSize: 11
                            font.bold: true
                            font.family: "Inter"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 7
                        radius: 3.5
                        color: Qt.rgba(1, 1, 1, 0.08)

                        Rectangle {
                            height: parent.height
                            width: root.totalTasks > 0 ? (parent.width * (root.inProgressTasks / root.totalTasks)) : 0
                            radius: 3.5
                            color: "#F59E0B"
                            Behavior on width { NumberAnimation { duration: 250; easing.type: Easing.OutQuad } }
                        }
                    }
                }

                // 3. To-Do / Pending Bar
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "To-Do / Pending (" + root.pendingTasks + " tasks)"
                            color: "#E2E8F0"
                            font.pixelSize: 11
                            font.weight: Font.Medium
                            font.family: "Inter"
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: (root.totalTasks > 0 ? Math.round((root.pendingTasks * 100) / root.totalTasks) : 0) + "%"
                            color: "#3B82F6"
                            font.pixelSize: 11
                            font.bold: true
                            font.family: "Inter"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 7
                        radius: 3.5
                        color: Qt.rgba(1, 1, 1, 0.08)

                        Rectangle {
                            height: parent.height
                            width: root.totalTasks > 0 ? (parent.width * (root.pendingTasks / root.totalTasks)) : 0
                            radius: 3.5
                            color: "#3B82F6"
                            Behavior on width { NumberAnimation { duration: 250; easing.type: Easing.OutQuad } }
                        }
                    }
                }
            }
        }
    }
}
