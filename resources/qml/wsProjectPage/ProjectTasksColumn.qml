import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: "#161824"
    radius: 12
    border.color: Qt.rgba(1, 1, 1, 0.08)
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // --- Column Header ---
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "Project Tasks & Milestones"
                color: "#FFFFFF"
                font.pixelSize: 16
                font.bold: true
                font.family: "Inter"
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                height: 22
                width: taskCountText.implicitWidth + 14
                radius: 11
                color: Qt.rgba(1, 1, 1, 0.06)

                Text {
                    id: taskCountText
                    anchors.centerIn: parent
                    text: (typeof wsProjectPage !== "undefined" ? wsProjectPage.tasks.length : 0) + " tasks"
                    color: "#94A3B8"
                    font.pixelSize: 11
                    font.family: "Inter"
                }
            }
        }

        // --- Inline Quick Add Task Row ---
        Rectangle {
            Layout.fillWidth: true
            height: 38
            radius: 8
            color: "#1C1E2B"
            border.color: newTaskInput.activeFocus ? "#8B5CF6" : Qt.rgba(1, 1, 1, 0.08)
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 8
                spacing: 8

                Text {
                    text: "+"
                    color: "#A78BFA"
                    font.pixelSize: 16
                    font.bold: true
                }

                TextInput {
                    id: newTaskInput
                    Layout.fillWidth: true
                    color: "#FFFFFF"
                    font.pixelSize: 12
                    font.family: "Inter"
                    clip: true

                    Text {
                        text: "Add a task to this project and press Enter..."
                        color: "#64748B"
                        font.pixelSize: 12
                        font.family: "Inter"
                        visible: !newTaskInput.text && !newTaskInput.activeFocus
                    }

                    onAccepted: {
                        if (text.trim().length > 0 && typeof wsProjectPage !== "undefined") {
                            wsProjectPage.createNewTask(text.trim(), "Medium")
                            text = ""
                        }
                    }
                }

                Rectangle {
                    width: 48
                    height: 24
                    radius: 5
                    color: newTaskInput.text.trim().length > 0 ? "#8B5CF6" : Qt.rgba(1, 1, 1, 0.05)
                    visible: newTaskInput.text.trim().length > 0

                    Text {
                        anchors.centerIn: parent
                        text: "Add"
                        color: "#FFFFFF"
                        font.pixelSize: 11
                        font.bold: true
                        font.family: "Inter"
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (newTaskInput.text.trim().length > 0 && typeof wsProjectPage !== "undefined") {
                                wsProjectPage.createNewTask(newTaskInput.text.trim(), "Medium")
                                newTaskInput.text = ""
                            }
                        }
                    }
                }
            }
        }

        // --- Empty State ---
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: typeof wsProjectPage !== "undefined" && wsProjectPage.tasks.length === 0

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "🎯"
                    font.pixelSize: 32
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "No tasks created yet"
                    color: "#FFFFFF"
                    font.pixelSize: 14
                    font.bold: true
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Type a task above or click '+ New Task' to begin."
                    color: "#94A3B8"
                    font.pixelSize: 12
                }
            }
        }

        // --- Task List View ---
        ListView {
            id: taskListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8
            clip: true
            visible: typeof wsProjectPage !== "undefined" && wsProjectPage.tasks.length > 0
            model: typeof wsProjectPage !== "undefined" ? wsProjectPage.tasks : []

            delegate: Rectangle {
                id: taskItemRect
                width: taskListView.width
                height: 52
                radius: 8
                color: taskMouse.containsMouse ? "#232637" : "#1B1D2B"
                border.color: taskMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.16) : Qt.rgba(1, 1, 1, 0.06)
                border.width: 1

                Behavior on color { ColorAnimation { duration: 120 } }
                Behavior on border.color { ColorAnimation { duration: 120 } }

                property string taskId: String(modelData.id || "")
                property bool isDone: Boolean(modelData.isCompleted)
                property string priorityLevel: String(modelData.priority || "Medium")

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    // Checkbox
                    Rectangle {
                        width: 20
                        height: 20
                        radius: 5
                        color: taskItemRect.isDone ? "#10B981" : "transparent"
                        border.color: taskItemRect.isDone ? "#10B981" : (checkArea.containsMouse ? "#A78BFA" : "#4B5563")
                        border.width: 1.5

                        Text {
                            anchors.centerIn: parent
                            text: "✓"
                            color: "#FFFFFF"
                            font.pixelSize: 13
                            font.bold: true
                            visible: taskItemRect.isDone
                        }

                        MouseArea {
                            id: checkArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (typeof wsProjectPage !== "undefined" && taskItemRect.taskId) {
                                    wsProjectPage.toggleTaskCompletion(taskItemRect.taskId, !taskItemRect.isDone)
                                }
                            }
                        }
                    }

                    // Priority Badge
                    Rectangle {
                        height: 20
                        width: prioText.implicitWidth + 12
                        radius: 4
                        color: {
                            if (taskItemRect.priorityLevel === "High") return Qt.rgba(0.94, 0.27, 0.27, 0.18)
                            if (taskItemRect.priorityLevel === "Low") return Qt.rgba(0.23, 0.51, 0.96, 0.18)
                            return Qt.rgba(0.96, 0.62, 0.04, 0.18)
                        }

                        Text {
                            id: prioText
                            anchors.centerIn: parent
                            text: taskItemRect.priorityLevel
                            font.pixelSize: 10
                            font.bold: true
                            font.family: "Inter"
                            color: {
                                if (taskItemRect.priorityLevel === "High") return "#F87171"
                                if (taskItemRect.priorityLevel === "Low") return "#60A5FA"
                                return "#FBBF24"
                            }
                        }
                    }

                    // Task Title
                    Text {
                        text: modelData.title || "Untitled Task"
                        color: taskItemRect.isDone ? "#64748B" : "#FFFFFF"
                        font.pixelSize: 13
                        font.weight: taskItemRect.isDone ? Font.Normal : Font.Medium
                        font.strikeout: taskItemRect.isDone
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        font.family: "Inter"
                    }

                    // Due Date Label
                    Text {
                        visible: modelData.dueDateFormatted !== ""
                        text: modelData.dueDateFormatted || ""
                        color: "#64748B"
                        font.pixelSize: 11
                        font.family: "Inter"
                    }

                    // Delete Button on hover
                    Text {
                        text: "✕"
                        font.pixelSize: 11
                        color: "#94A3B8"
                        visible: taskMouse.containsMouse

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (typeof wsProjectPage !== "undefined" && taskItemRect.taskId) {
                                    wsProjectPage.deleteTask(taskItemRect.taskId)
                                }
                            }
                        }
                    }
                }

                MouseArea {
                    id: taskMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    z: -1
                }
            }
        }
    }
}
