import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: delegateRoot
    width: ListView.view ? ListView.view.width : 280
    height: cardRoot.implicitHeight + 6
    z: cardRoot.Drag.active ? 9999 : 1

    property string taskId: modelData.id || ""
    property int currentStatus: modelData.status !== undefined ? modelData.status : 0
    property bool subtasksExpanded: false

    Rectangle {
        id: cardRoot
        width: delegateRoot.width
        implicitHeight: mainLayout.implicitHeight + 24
        z: Drag.active ? 9999 : 1

        color: hoverHandler.hovered ? "#1C2433" : "#161C28"
        radius: 10
        border.color: Drag.active ? "#6366F1" : (hoverHandler.hovered ? "#3B4760" : "#242C3F")
        border.width: Drag.active ? 2 : 1

        scale: Drag.active ? 1.04 : 1.0
        opacity: Drag.active ? 0.9 : 1.0
        Behavior on scale {
            NumberAnimation {
                duration: 100
            }
        }
        Behavior on opacity {
            NumberAnimation {
                duration: 100
            }
        }

        property string taskId: delegateRoot.taskId
        property int currentStatus: delegateRoot.currentStatus

        // Drag configuration
        Drag.active: dragArea.drag.active
        Drag.source: cardRoot
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
        Drag.keys: ["task-card"]

        HoverHandler {
            id: hoverHandler
        }

        // Right-click context menu
        Menu {
            id: contextMenu
            background: Rectangle {
                color: "#161C28"
                border.color: "#2A3348"
                border.width: 1
                radius: 8
            }

            MenuItem {
                text: "🔵 Move to To Do"
                visible: cardRoot.currentStatus !== 0
                contentItem: Text {
                    text: parent.text
                    color: "#93C5FD"
                    font.pixelSize: 12
                }
                onTriggered: {
                    if (typeof taskBoard !== "undefined") {
                        taskBoard.updateTaskStatus(cardRoot.taskId, 0)
                    }
                }
            }

            MenuItem {
                text: "🟠 Move to In Progress"
                visible: cardRoot.currentStatus !== 1
                contentItem: Text {
                    text: parent.text
                    color: "#FBBF24"
                    font.pixelSize: 12
                }
                onTriggered: {
                    if (typeof taskBoard !== "undefined") {
                        taskBoard.updateTaskStatus(cardRoot.taskId, 1)
                    }
                }
            }

            MenuItem {
                text: "🟢 Move to Completed"
                visible: cardRoot.currentStatus !== 2
                contentItem: Text {
                    text: parent.text
                    color: "#34D399"
                    font.pixelSize: 12
                }
                onTriggered: {
                    if (typeof taskBoard !== "undefined") {
                        taskBoard.updateTaskStatus(cardRoot.taskId, 2)
                    }
                }
            }

            MenuItem {
                text: delegateRoot.subtasksExpanded ? "▲ Hide Subtasks" : "▼ View / Add Subtasks"
                contentItem: Text {
                    text: parent.text
                    color: "#C7D2FE"
                    font.pixelSize: 12
                }
                onTriggered: {
                    delegateRoot.subtasksExpanded = !delegateRoot.subtasksExpanded
                }
            }

            MenuSeparator {
                contentItem: Rectangle {
                    implicitHeight: 1
                    color: "#2A3348"
                }
            }

            MenuItem {
                text: "🗑 Delete Task"
                contentItem: Text {
                    text: parent.text
                    color: "#F87171"
                    font.pixelSize: 12
                }
                onTriggered: {
                    if (typeof taskBoard !== "undefined") {
                        taskBoard.deleteTask(cardRoot.taskId)
                    }
                }
            }
        }

        // Right click mouse area for entire card
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: (mouse) => {
                if (mouse.button === Qt.RightButton) {
                    contextMenu.popup()
                }
            }
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

                // 6-dot drag grip handle with drag area
                Item {
                    width: 16
                    height: 20
                    Layout.alignment: Qt.AlignVCenter

                    Text {
                        anchors.centerIn: parent
                        text: "⠿"
                        color: dragArea.drag.active ? "#6366F1" : (dragArea.containsMouse ? "#FFFFFF" : "#64748B")
                        font.pixelSize: 15
                    }

                    MouseArea {
                        id: dragArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: drag.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                        drag.target: cardRoot
                        drag.axis: Drag.XAndYAxis
                        onReleased: {
                            cardRoot.Drag.drop()
                            cardRoot.x = 0
                            cardRoot.y = 0
                        }
                    }
                }

                // Checkbox
                Rectangle {
                    width: 18
                    height: 18
                    radius: 4
                    color: (cardRoot.currentStatus === 2) ? "#6366F1" : "#1E2538"
                    border.color: (cardRoot.currentStatus === 2) ? "#6366F1" : "#475569"
                    border.width: 1.5

                    Text {
                        anchors.centerIn: parent
                        text: "✓"
                        color: "#FFFFFF"
                        font.pixelSize: 11
                        font.bold: true
                        visible: cardRoot.currentStatus === 2
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (typeof taskBoard !== "undefined") {
                                taskBoard.toggleTaskCompletion(cardRoot.taskId, cardRoot.currentStatus !== 2)
                            }
                        }
                    }
                }

                // Title
                Text {
                    text: modelData.title || ""
                    color: (cardRoot.currentStatus === 2) ? "#94A3B8" : "#F1F5F9"
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                    font.strikeout: (cardRoot.currentStatus === 2)
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
                                taskBoard.deleteTask(cardRoot.taskId)
                            }
                        }
                    }
                }
            }

            // Priority Badge Row + Quick Move Action Buttons
            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                // Priority Pill
                Rectangle {
                    id: prioPill
                    visible: modelData.priorityText !== undefined
                    implicitWidth: prioLabel.implicitWidth + 14
                    implicitHeight: 20
                    radius: 10

                    readonly property color pillColor: {
                        switch (modelData.priority) {
                            case 0:
                                return "#38BDF8"; // Low (Sky blue)
                            case 1:
                                return "#F59E0B"; // Medium (Amber)
                            case 2:
                                return "#EF4444"; // High (Red)
                            case 3:
                                return "#A855F7"; // Critical (Purple)
                            default:
                                return "#F59E0B";
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

                Item {
                    Layout.fillWidth: true
                }

                // Quick Move: "Start" (To In Progress)
                Rectangle {
                    visible: cardRoot.currentStatus === 0
                    implicitWidth: btnStartText.implicitWidth + 12
                    implicitHeight: 20
                    radius: 4
                    color: btnStartMouse.containsMouse ? "#D97706" : "#241F15"
                    border.color: "#B45309"
                    border.width: 1

                    Text {
                        id: btnStartText
                        anchors.centerIn: parent
                        text: "▶ Start"
                        color: btnStartMouse.containsMouse ? "#FFFFFF" : "#F59E0B"
                        font.pixelSize: 10
                        font.bold: true
                    }

                    MouseArea {
                        id: btnStartMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (typeof taskBoard !== "undefined") {
                                taskBoard.updateTaskStatus(cardRoot.taskId, 1)
                            }
                        }
                    }
                }

                // Quick Move: In Progress -> "Done" or "Back to To Do"
                RowLayout {
                    visible: cardRoot.currentStatus === 1
                    spacing: 4

                    Rectangle {
                        implicitWidth: btnBackText.implicitWidth + 8
                        implicitHeight: 20
                        radius: 4
                        color: btnBackMouse.containsMouse ? "#2563EB" : "#172033"
                        border.color: "#1D4ED8"
                        border.width: 1

                        Text {
                            id: btnBackText
                            anchors.centerIn: parent
                            text: "◀ To Do"
                            color: btnBackMouse.containsMouse ? "#FFFFFF" : "#93C5FD"
                            font.pixelSize: 10
                        }

                        MouseArea {
                            id: btnBackMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (typeof taskBoard !== "undefined") {
                                    taskBoard.updateTaskStatus(cardRoot.taskId, 0)
                                }
                            }
                        }
                    }

                    Rectangle {
                        implicitWidth: btnDoneText.implicitWidth + 8
                        implicitHeight: 20
                        radius: 4
                        color: btnDoneMouse.containsMouse ? "#059669" : "#142921"
                        border.color: "#047857"
                        border.width: 1

                        Text {
                            id: btnDoneText
                            anchors.centerIn: parent
                            text: "✓ Done"
                            color: btnDoneMouse.containsMouse ? "#FFFFFF" : "#34D399"
                            font.pixelSize: 10
                            font.bold: true
                        }

                        MouseArea {
                            id: btnDoneMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (typeof taskBoard !== "undefined") {
                                    taskBoard.updateTaskStatus(cardRoot.taskId, 2)
                                }
                            }
                        }
                    }
                }

                // Quick Move: Completed -> "Reopen"
                Rectangle {
                    visible: cardRoot.currentStatus === 2
                    implicitWidth: btnReopenText.implicitWidth + 10
                    implicitHeight: 20
                    radius: 4
                    color: btnReopenMouse.containsMouse ? "#242C3F" : "#1A202C"
                    border.color: "#334155"
                    border.width: 1

                    Text {
                        id: btnReopenText
                        anchors.centerIn: parent
                        text: "↩ Reopen"
                        color: btnReopenMouse.containsMouse ? "#FFFFFF" : "#94A3B8"
                        font.pixelSize: 10
                    }

                    MouseArea {
                        id: btnReopenMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (typeof taskBoard !== "undefined") {
                                taskBoard.updateTaskStatus(cardRoot.taskId, 0)
                            }
                        }
                    }
                }
            }

            // Due Date / Finished Date Row
            Text {
                Layout.fillWidth: true
                visible: text.length > 0
                text: {
                    if (cardRoot.currentStatus === 2 && modelData.finishedText && modelData.finishedText.length > 0) {
                        return modelData.finishedText;
                    }
                    return modelData.dueStatusText || "";
                }
                color: modelData.isOverdue ? "#F87171" : "#94A3B8"
                font.pixelSize: 11
                font.weight: modelData.isOverdue ? Font.DemiBold : Font.Normal
            }

            // Footer: Notes Count + Subtasks progress toggle + Assignee Avatar
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 4
                spacing: 12

                // Linked Notes Indicator
                RowLayout {
                    spacing: 4
                    visible: modelData.linkedNotesCount !== undefined && modelData.linkedNotesCount > 0

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

                // Subtask Progress Indicator & Toggle Button
                Rectangle {
                    implicitWidth: subtaskBadgeLayout.implicitWidth + 12
                    implicitHeight: 22
                    radius: 4
                    color: subtaskBadgeArea.containsMouse ? "#242C3F" : (delegateRoot.subtasksExpanded ? "#1E2538" : "transparent")
                    border.color: delegateRoot.subtasksExpanded ? "#3B4760" : "transparent"
                    border.width: 1

                    RowLayout {
                        id: subtaskBadgeLayout
                        anchors.centerIn: parent
                        spacing: 4

                        Text {
                            text: delegateRoot.subtasksExpanded ? "▼" : "▶"
                            font.pixelSize: 8
                            color: "#64748B"
                        }

                        Text {
                            text: modelData.allSubtasksDone ? "✓" : "○"
                            color: modelData.allSubtasksDone ? "#22C55E" : "#94A3B8"
                            font.pixelSize: 11
                            font.bold: true
                        }

                        Text {
                            text: {
                                if (modelData.hasSubtasks) {
                                    return (modelData.subtasksCompleted || 0) + "/" + (modelData.subtasksTotal || 0);
                                }
                                return "Subtasks";
                            }
                            color: modelData.allSubtasksDone ? "#22C55E" : "#94A3B8"
                            font.pixelSize: 11
                            font.weight: Font.Medium
                        }
                    }

                    MouseArea {
                        id: subtaskBadgeArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: delegateRoot.subtasksExpanded = !delegateRoot.subtasksExpanded
                    }
                }

                Item {
                    Layout.fillWidth: true
                }


            }

            // Expandable Subtasks Checklist Section
            ColumnLayout {
                Layout.fillWidth: true
                visible: delegateRoot.subtasksExpanded
                spacing: 8

                // Subtle divider line
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#242C3F"
                }

                // Subtask Items List
                Repeater {
                    model: modelData.subtasks || []

                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        // Small Checkbox
                        Rectangle {
                            width: 15
                            height: 15
                            radius: 3
                            color: modelData.isCompleted ? "#6366F1" : "#1E2538"
                            border.color: modelData.isCompleted ? "#6366F1" : "#475569"
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "✓"
                                color: "#FFFFFF"
                                font.pixelSize: 10
                                font.bold: true
                                visible: modelData.isCompleted
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (typeof taskBoard !== "undefined") {
                                        taskBoard.toggleSubtask(delegateRoot.taskId, modelData.id, !modelData.isCompleted)
                                    }
                                }
                            }
                        }

                        // Subtask Title
                        Text {
                            text: modelData.title || ""
                            color: modelData.isCompleted ? "#64748B" : "#CBD5E1"
                            font.pixelSize: 12
                            font.strikeout: modelData.isCompleted
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }

                        // Delete Subtask Button
                        Text {
                            text: "✕"
                            color: deleteSubtaskArea.containsMouse ? "#EF4444" : "#64748B"
                            font.pixelSize: 11

                            MouseArea {
                                id: deleteSubtaskArea
                                anchors.fill: parent
                                anchors.margins: -4
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (typeof taskBoard !== "undefined") {
                                        taskBoard.deleteSubtask(delegateRoot.taskId, modelData.id)
                                    }
                                }
                            }
                        }
                    }
                }

                // Inline "+ Add Subtask" Input Row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 28
                        radius: 5
                        color: "#111520"
                        border.color: newSubtaskInput.activeFocus ? "#6366F1" : "#2A3348"
                        border.width: 1

                        TextInput {
                            id: newSubtaskInput
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            anchors.topMargin: 4
                            anchors.bottomMargin: 4
                            color: "#FFFFFF"
                            font.pixelSize: 11
                            selectByMouse: true

                            Text {
                                anchors.fill: parent
                                text: "+ Add subtask... (Enter)"
                                color: "#475569"
                                font.pixelSize: 11
                                visible: !newSubtaskInput.text && !newSubtaskInput.activeFocus
                            }

                            onAccepted: {
                                var clean = text.trim();
                                if (clean !== "" && typeof taskBoard !== "undefined") {
                                    taskBoard.addSubtask(delegateRoot.taskId, clean);
                                    text = "";
                                }
                            }
                        }
                    }

                    Rectangle {
                        visible: newSubtaskInput.text.trim().length > 0
                        implicitWidth: 38
                        implicitHeight: 28
                        radius: 5
                        color: addBtnMouse.containsMouse ? "#4F46E5" : "#6366F1"

                        Text {
                            anchors.centerIn: parent
                            text: "Add"
                            color: "#FFFFFF"
                            font.pixelSize: 10
                            font.bold: true
                        }

                        MouseArea {
                            id: addBtnMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                var clean = newSubtaskInput.text.trim();
                                if (clean !== "" && typeof taskBoard !== "undefined") {
                                    taskBoard.addSubtask(delegateRoot.taskId, clean);
                                    newSubtaskInput.text = "";
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
