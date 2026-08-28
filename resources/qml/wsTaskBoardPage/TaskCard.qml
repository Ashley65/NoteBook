import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: cardRoot
    width: ListView.view ? ListView.view.width : 280
    implicitHeight: mainLayout.implicitHeight + 24

    color: "#161C28"
    radius: 10
    border.color: hoverHandler.hovered ? "#3B4760" : "#242C3F"
    border.width: 1

    HoverHandler {
        id: hoverHandler
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Top Row: Drag Handle + Checkbox + Title + Delete
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            // 6-dot drag grip handle
            Text {
                text: "⠿"
                color: "#64748B"
                font.pixelSize: 15
                Layout.alignment: Qt.AlignVCenter
            }

            // Checkbox
            Rectangle {
                width: 18
                height: 18
                radius: 4
                color: (modelData.status === 2) ? "#6366F1" : "#1E2538"
                border.color: (modelData.status === 2) ? "#6366F1" : "#475569"
                border.width: 1.5

                Text {
                    anchors.centerIn: parent
                    text: "✓"
                    color: "#FFFFFF"
                    font.pixelSize: 11
                    font.bold: true
                    visible: modelData.status === 2
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (typeof taskBoard !== "undefined") {
                            taskBoard.toggleTaskCompletion(modelData.id, modelData.status !== 2)
                        }
                    }
                }
            }

            // Title
            Text {
                text: modelData.title || ""
                color: (modelData.status === 2) ? "#94A3B8" : "#F1F5F9"
                font.pixelSize: 13
                font.weight: Font.DemiBold
                font.strikeout: (modelData.status === 2)
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            // Delete 'x' Button
            Text {
                text: "✕"
                color: deleteArea.containsMouse ? "#EF4444" : "#64748B"
                font.pixelSize: 12
                Layout.alignment: Qt.AlignVCenter

                MouseArea {
                    id: deleteArea
                    anchors.fill: parent
                    anchors.margins: -4
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (typeof taskBoard !== "undefined") {
                            taskBoard.deleteTask(modelData.id)
                        }
                    }
                }
            }
        }

        // Priority Badge Row
        RowLayout {
            Layout.fillWidth: true
            visible: modelData.priorityText !== undefined

            Rectangle {
                id: prioPill
                implicitWidth: prioLabel.implicitWidth + 14
                implicitHeight: 20
                radius: 10

                // Dynamic coloring for Priority
                readonly property color pillColor: {
                    switch (modelData.priority) {
                        case 0: return "#38BDF8"; // Low (Sky blue)
                        case 1: return "#F59E0B"; // Medium (Amber)
                        case 2: return "#EF4444"; // High (Red)
                        case 3: return "#A855F7"; // Critical (Purple)
                        default: return "#F59E0B";
                    }
                }

                color: Qt.rgba(pillColor.r, pillColor.g, pillColor.b, 0.15)
                border.color: Qt.rgba(pillColor.r, pillColor.g, pillColor.b, 0.3)
                border.width: 1

                Text {
                    id: prioLabel
                    anchors.centerIn: parent
                    text: modelData.priorityText || "Medium"
                    color: prioPill.pillColor
                    font.pixelSize: 10
                    font.bold: true
                }
            }
        }

        // Due Date / Finished Date Row
        Text {
            Layout.fillWidth: true
            visible: text.length > 0
            text: {
                if (modelData.status === 2 && modelData.finishedText && modelData.finishedText.length > 0) {
                    return modelData.finishedText;
                }
                return modelData.dueStatusText || "";
            }
            color: modelData.isOverdue ? "#F87171" : "#94A3B8"
            font.pixelSize: 11
            font.weight: modelData.isOverdue ? Font.DemiBold : Font.Normal
        }

        // Footer: Notes Count + Subtasks progress + Assignee Avatar
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 4
            spacing: 12

            // Linked Notes Indicator
            RowLayout {
                spacing: 4
                visible: modelData.linkedNotesCount !== undefined

                Text {
                    text: "📎"
                    font.pixelSize: 11
                }
                Text {
                    text: (modelData.linkedNotesCount || 0) + (modelData.linkedNotesCount === 1 ? " Note" : " Notes")
                    color: "#94A3B8"
                    font.pixelSize: 11
                }
            }

            // Subtask Progress Indicator
            RowLayout {
                spacing: 4
                visible: modelData.hasSubtasks === true

                Text {
                    text: modelData.allSubtasksDone ? "✓" : "○"
                    color: modelData.allSubtasksDone ? "#22C55E" : "#94A3B8"
                    font.pixelSize: 11
                    font.bold: true
                }
                Text {
                    text: (modelData.subtasksCompleted || 0) + "/" + (modelData.subtasksTotal || 0)
                    color: modelData.allSubtasksDone ? "#22C55E" : "#94A3B8"
                    font.pixelSize: 11
                    font.weight: Font.Medium
                }
            }

            Item {
                Layout.fillWidth: true
            }

            // Assignee Avatar
            Rectangle {
                width: 22
                height: 22
                radius: 11
                color: "#312E81"
                border.color: "#4338CA"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "👤"
                    font.pixelSize: 10
                }
            }
        }
    }
}
