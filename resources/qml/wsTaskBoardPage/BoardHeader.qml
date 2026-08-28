import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    id: headerRoot
    spacing: 16

    // Top Title & Subtitle
    ColumnLayout {
        spacing: 4

        RowLayout {
            spacing: 8

            Text {
                text: "Tasks & Kanban Board"
                color: "#FFFFFF"
                font.pixelSize: 22
                font.bold: true
            }

            Text {
                visible: typeof taskBoard !== "undefined" && taskBoard.activeProjectName !== ""
                text: "— " + (typeof taskBoard !== "undefined" ? taskBoard.activeProjectName : "")
                color: "#94A3B8"
                font.pixelSize: 20
                font.weight: Font.DemiBold
            }
        }

        Text {
            text: "Manage tasks, assign priority levels, and track deadlines."
            color: "#64748B"
            font.pixelSize: 13
        }
    }

    // Controls Row: Search + Filter + Sort + Add Task
    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        // Search Bar
        Rectangle {
            Layout.preferredWidth: 320
            Layout.preferredHeight: 36
            radius: 8
            color: "#131824"
            border.color: searchInput.activeFocus ? "#6366F1" : "#242C3F"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 8

                Text {
                    text: "🔍"
                    font.pixelSize: 12
                    color: "#64748B"
                }

                TextInput {
                    id: searchInput
                    Layout.fillWidth: true
                    color: "#F1F5F9"
                    font.pixelSize: 13
                    selectByMouse: true
                    clip: true

                    Text {
                        anchors.fill: parent
                        text: "Search tasks, linked notes..."
                        color: "#475569"
                        font.pixelSize: 13
                        visible: !searchInput.text && !searchInput.activeFocus
                    }

                    onTextChanged: {
                        if (typeof taskBoard !== "undefined") {
                            taskBoard.searchQuery = text
                        }
                    }
                }

                Text {
                    text: "✕"
                    color: "#64748B"
                    font.pixelSize: 11
                    visible: searchInput.text.length > 0
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: searchInput.text = ""
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }

        // Filter Dropdown
        RowLayout {
            spacing: 6

            Text {
                text: "Filter:"
                color: "#64748B"
                font.pixelSize: 12
            }

            ComboBox {
                id: filterCombo
                model: ["All Priorities", "Low", "Medium", "High", "Critical"]
                currentIndex: 0
                implicitHeight: 36
                implicitWidth: 130

                background: Rectangle {
                    color: "#131824"
                    border.color: "#242C3F"
                    border.width: 1
                    radius: 8
                }

                contentItem: Text {
                    leftPadding: 10
                    text: filterCombo.displayText
                    color: "#E2E8F0"
                    font.pixelSize: 12
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                onCurrentIndexChanged: {
                    if (typeof taskBoard !== "undefined") {
                        // 0 = All (-1), 1 = Low (0), 2 = Medium (1), 3 = High (2), 4 = Critical (3)
                        taskBoard.priorityFilter = currentIndex - 1;
                    }
                }
            }
        }

        // Sort Dropdown
        RowLayout {
            spacing: 6

            Text {
                text: "Sort:"
                color: "#64748B"
                font.pixelSize: 12
            }

            ComboBox {
                id: sortCombo
                model: ["Due Date (Earliest)", "Priority", "Title (A-Z)", "Newest Created"]
                currentIndex: 0
                implicitHeight: 36
                implicitWidth: 160

                background: Rectangle {
                    color: "#131824"
                    border.color: "#242C3F"
                    border.width: 1
                    radius: 8
                }

                contentItem: Text {
                    leftPadding: 10
                    text: sortCombo.displayText
                    color: "#E2E8F0"
                    font.pixelSize: 12
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                onCurrentIndexChanged: {
                    if (typeof taskBoard !== "undefined") {
                        taskBoard.sortMode = currentIndex;
                    }
                }
            }
        }

        // + Add Task Button
        Button {
            id: btnAddTask
            text: "+ Add Task"
            implicitHeight: 36
            implicitWidth: 110

            contentItem: RowLayout {
                spacing: 4
                anchors.centerIn: parent

                Text {
                    text: "+"
                    color: "#FFFFFF"
                    font.pixelSize: 14
                    font.bold: true
                }
                Text {
                    text: "Add Task"
                    color: "#FFFFFF"
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                }
            }

            background: Rectangle {
                color: btnAddTask.hovered ? "#4F46E5" : "#6366F1"
                radius: 8

                Behavior on color {
                    ColorAnimation { duration: 150 }
                }
            }

            onClicked: addTaskDialog.open()
        }
    }

    // Add Task Dialog Popup
    Dialog {
        id: addTaskDialog
        title: "Create New Task"
        modal: true
        anchors.centerIn: parent
        width: 380
        height: 400
        padding: 20

        background: Rectangle {
            color: "#161C28"
            border.color: "#2A3348"
            border.width: 1
            radius: 12
        }

        header: Text {
            text: "Create New Task"
            color: "#FFFFFF"
            font.pixelSize: 16
            font.bold: true
            padding: 16
            bottomPadding: 0
        }

        contentItem: ColumnLayout {
            spacing: 12

            Text {
                text: "Task Title"
                color: "#94A3B8"
                font.pixelSize: 12
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 36
                radius: 6
                color: "#111520"
                border.color: taskTitleInput.activeFocus ? "#6366F1" : "#2A3348"

                TextInput {
                    id: taskTitleInput
                    anchors.fill: parent
                    anchors.margins: 8
                    color: "#FFFFFF"
                    font.pixelSize: 13
                    selectByMouse: true
                }
            }

            Text {
                text: "Description (Optional)"
                color: "#94A3B8"
                font.pixelSize: 12
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 60
                radius: 6
                color: "#111520"
                border.color: taskDescInput.activeFocus ? "#6366F1" : "#2A3348"

                TextArea {
                    id: taskDescInput
                    anchors.fill: parent
                    anchors.margins: 4
                    color: "#FFFFFF"
                    font.pixelSize: 12
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                }
            }

            RowLayout {
                spacing: 12

                ColumnLayout {
                    Layout.fillWidth: true

                    Text {
                        text: "Priority"
                        color: "#94A3B8"
                        font.pixelSize: 12
                    }

                    ComboBox {
                        id: newPrioCombo
                        Layout.fillWidth: true
                        implicitHeight: 36
                        model: ["Low", "Medium", "High", "Critical"]
                        currentIndex: 1 // Default Medium

                        background: Rectangle {
                            color: "#111520"
                            border.color: "#2A3348"
                            radius: 6
                        }
                        contentItem: Text {
                            leftPadding: 10
                            text: newPrioCombo.displayText
                            color: "#FFFFFF"
                            font.pixelSize: 12
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    Text {
                        text: "Due Date"
                        color: "#94A3B8"
                        font.pixelSize: 12
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 36
                        radius: 6
                        color: "#111520"
                        border.color: dueDateInput.activeFocus ? "#6366F1" : "#2A3348"

                        TextInput {
                            id: dueDateInput
                            anchors.fill: parent
                            anchors.margins: 8
                            color: "#FFFFFF"
                            font.pixelSize: 13
                            selectByMouse: true

                            Text {
                                anchors.fill: parent
                                text: "YYYY-MM-DD (Optional)"
                                color: "#475569"
                                font.pixelSize: 13
                                visible: !dueDateInput.text && !dueDateInput.activeFocus
                            }
                        }
                    }

                    // Quick date chips
                    RowLayout {
                        spacing: 6
                        Layout.fillWidth: true

                        function formatDate(d) {
                            var year = d.getFullYear();
                            var month = (d.getMonth() + 1).toString().padStart(2, '0');
                            var day = d.getDate().toString().padStart(2, '0');
                            return year + "-" + month + "-" + day;
                        }

                        Repeater {
                            model: [
                                { label: "Today", days: 0 },
                                { label: "Tomorrow", days: 1 },
                                { label: "+1 Wk", days: 7 }
                            ]

                            delegate: Rectangle {
                                implicitWidth: chipText.implicitWidth + 10
                                implicitHeight: 22
                                radius: 4
                                color: chipMouse.containsMouse ? "#242C3F" : "#1A202C"
                                border.color: "#2D3748"
                                border.width: 1

                                Text {
                                    id: chipText
                                    anchors.centerIn: parent
                                    text: modelData.label
                                    color: "#94A3B8"
                                    font.pixelSize: 10
                                }

                                MouseArea {
                                    id: chipMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        var target = new Date();
                                        target.setDate(target.getDate() + modelData.days);
                                        dueDateInput.text = parent.parent.formatDate(target);
                                    }
                                }
                            }
                        }

                        // Clear Date Chip
                        Rectangle {
                            visible: dueDateInput.text.length > 0
                            implicitWidth: clearChipText.implicitWidth + 10
                            implicitHeight: 22
                            radius: 4
                            color: clearChipMouse.containsMouse ? "#3B1D22" : "#24171A"
                            border.color: "#5C242A"
                            border.width: 1

                            Text {
                                id: clearChipText
                                anchors.centerIn: parent
                                text: "Clear"
                                color: "#F87171"
                                font.pixelSize: 10
                            }

                            MouseArea {
                                id: clearChipMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: dueDateInput.text = ""
                            }
                        }
                    }
                }
            }

        }

        footer: DialogButtonBox {
            background: Rectangle {
                color: "transparent"
            }
            alignment: Qt.AlignRight

            Button {
                text: "Cancel"
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                background: Rectangle {
                    color: "#1E2538"
                    radius: 6
                    implicitHeight: 32
                    implicitWidth: 70
                }
                contentItem: Text {
                    text: "Cancel"
                    color: "#94A3B8"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    dueDateInput.text = ""
                    addTaskDialog.reject()
                }
            }

            Button {
                text: "Create"
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                background: Rectangle {
                    color: "#6366F1"
                    radius: 6
                    implicitHeight: 32
                    implicitWidth: 70
                }
                contentItem: Text {
                    text: "Create"
                    color: "#FFFFFF"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    if (taskTitleInput.text.trim() !== "") {
                        var parsedDue = undefined;
                        var rawDue = dueDateInput.text.trim();
                        if (rawDue !== "") {
                            var parts = rawDue.split("-");
                            if (parts.length === 3) {
                                var y = parseInt(parts[0], 10);
                                var m = parseInt(parts[1], 10) - 1;
                                var d = parseInt(parts[2], 10);
                                var dt = new Date(y, m, d, 23, 59, 59);
                                if (!isNaN(dt.getTime())) {
                                    parsedDue = dt;
                                }
                            }
                        }

                        if (typeof taskBoard !== "undefined") {
                            if (parsedDue) {
                                taskBoard.createNewTask(taskTitleInput.text, taskDescInput.text, newPrioCombo.currentIndex, parsedDue);
                            } else {
                                taskBoard.createNewTask(taskTitleInput.text, taskDescInput.text, newPrioCombo.currentIndex);
                            }
                        }
                        taskTitleInput.text = ""
                        taskDescInput.text = ""
                        dueDateInput.text = ""
                        addTaskDialog.accept()
                    }
                }
            }
        }
    }
}
