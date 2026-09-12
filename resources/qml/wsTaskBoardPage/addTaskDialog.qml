import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import QtQuick.Window 2.15



ApplicationWindow {
    id: root
    visible: true
    width: 420
    height: 260
    title: "Create New Task"

    Button {
        text: "Open dialog"
        anchors.centerIn: parent
        onClicked: addTaskDialog.open()
    }

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
                            text: "YYYY-MM-DD"
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
                onClicked: addTaskDialog.reject()
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
                        if (typeof taskBoard !== "undefined") {
                            taskBoard.createNewTask(taskTitleInput.text, taskDescInput.text, newPrioCombo.currentIndex)
                        }
                        taskTitleInput.text = ""
                        taskDescInput.text = ""
                        addTaskDialog.accept()
                    }
                }
            }
        }
    }
}