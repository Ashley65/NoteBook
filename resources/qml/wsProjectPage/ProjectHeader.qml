import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    implicitHeight: mainLayout.implicitHeight
    Layout.fillWidth: true

    readonly property bool isCompact: root.width < 760

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        spacing: 16

        // Breadcrumb navigation
        RowLayout {
            spacing: 8
            Text {
                text: typeof wsProjectPage !== "undefined" ? wsProjectPage.workspaceName : "Workspace"
                color: "#64748B"
                font.pixelSize: 12
                font.family: "Inter"
            }
            Text {
                text: "/"
                color: "#475569"
                font.pixelSize: 12
            }
            Text {
                text: typeof wsProjectPage !== "undefined" ? wsProjectPage.projectName : "Project"
                color: "#94A3B8"
                font.pixelSize: 12
                font.weight: Font.Medium
                font.family: "Inter"
            }
        }

        // Header Main: Identity on Left, Buttons on Right
        GridLayout {
            Layout.fillWidth: true
            columns: root.isCompact ? 1 : 2
            rowSpacing: 12
            columnSpacing: 16

            // Left: Project Badge, Title, Description, Metadata
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                RowLayout {
                    spacing: 12
                    Layout.fillWidth: true

                    // Project Initials Badge
                    Rectangle {
                        height: 32
                        width: badgeText.implicitWidth + 16
                        radius: 8
                        color: typeof wsProjectPage !== "undefined"
                            ? Qt.rgba(0.55, 0.36, 0.96, 0.18)
                            : "#2A2045"
                        border.color: typeof wsProjectPage !== "undefined" ? wsProjectPage.projectColor : "#A78BFA"
                        border.width: 1.5

                        Text {
                            id: badgeText
                            anchors.centerIn: parent
                            text: typeof wsProjectPage !== "undefined" ? ("[" + wsProjectPage.projectInitials + "] " + wsProjectPage.projectName) : "[PR] Project"
                            color: typeof wsProjectPage !== "undefined" ? wsProjectPage.projectColor : "#A78BFA"
                            font.pixelSize: 12
                            font.bold: true
                            font.family: "Inter"
                        }
                    }

                    Text {
                        text: typeof wsProjectPage !== "undefined" ? ("Project: " + wsProjectPage.projectName) : "Project Overview"
                        color: "#FFFFFF"
                        font.pixelSize: root.isCompact ? 20 : 24
                        font.bold: true
                        font.family: "Inter"
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }

                Text {
                    text: typeof wsProjectPage !== "undefined" ? wsProjectPage.projectDescription : "Project documentation, tasks, and assets."
                    color: "#94A3B8"
                    font.pixelSize: 13
                    font.family: "Inter"
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }

                // Metadata Stats Pill Row
                RowLayout {
                    spacing: 16
                    Text {
                        text: "Created: " + (typeof wsProjectPage !== "undefined" ? wsProjectPage.createdAtFormatted : "Recently")
                        color: "#64748B"
                        font.pixelSize: 11
                        font.family: "Inter"
                    }
                    Text { text: "•"; color: "#475569"; font.pixelSize: 10 }
                    Text {
                        text: "Total Tasks: " + (typeof wsProjectPage !== "undefined" ? wsProjectPage.totalTasksCount : 0)
                        color: "#64748B"
                        font.pixelSize: 11
                        font.family: "Inter"
                    }
                    Text { text: "•"; color: "#475569"; font.pixelSize: 10 }
                    Text {
                        text: "Notes: " + (typeof wsProjectPage !== "undefined" ? wsProjectPage.notesCount : 0)
                        color: "#64748B"
                        font.pixelSize: 11
                        font.family: "Inter"
                    }
                    Text { text: "•"; color: "#475569"; font.pixelSize: 10 }
                    Text {
                        text: "Files: " + (typeof wsProjectPage !== "undefined" ? wsProjectPage.filesCount : 0)
                        color: "#64748B"
                        font.pixelSize: 11
                        font.family: "Inter"
                    }
                }
            }

            // Right: Action Buttons
            RowLayout {
                Layout.alignment: root.isCompact ? Qt.AlignLeft : Qt.AlignRight
                spacing: 8

                // + New Task Button
                Button {
                    id: btnNewTask
                    text: "+ New Task"
                    onClicked: {
                        if (typeof wsProjectPage !== "undefined") {
                            wsProjectPage.createNewTask("Untitled Task", "Medium")
                        }
                    }

                    contentItem: RowLayout {
                        spacing: 6
                        anchors.centerIn: parent
                        Text {
                            text: "+"
                            color: "#FFFFFF"
                            font.pixelSize: 15
                            font.bold: true
                        }
                        Text {
                            text: "New Task"
                            color: "#FFFFFF"
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            font.family: "Inter"
                        }
                    }

                    background: Rectangle {
                        implicitWidth: root.isCompact ? 108 : 118
                        implicitHeight: 36
                        radius: 8
                        color: btnNewTask.down ? "#6D28D9" : (btnNewTask.hovered ? "#7C3AED" : "#8B5CF6")
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }
                }

                // + New Note Button
                Button {
                    id: btnNewNote
                    text: "+ New Note"
                    onClicked: {
                        if (typeof wsProjectPage !== "undefined") {
                            wsProjectPage.createNewNote("Untitled Note")
                        }
                    }

                    contentItem: RowLayout {
                        spacing: 6
                        anchors.centerIn: parent
                        Text { text: "📄"; font.pixelSize: 11 }
                        Text {
                            text: "New Note"
                            color: "#E2E8F0"
                            font.pixelSize: 12
                            font.weight: Font.Medium
                            font.family: "Inter"
                        }
                    }

                    background: Rectangle {
                        implicitWidth: root.isCompact ? 104 : 114
                        implicitHeight: 36
                        radius: 8
                        color: btnNewNote.down ? "#161824" : (btnNewNote.hovered ? "#26293A" : "#1C1E2B")
                        border.color: btnNewNote.hovered ? "#474D67" : "#2E3245"
                        border.width: 1
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }
                }

                // Upload Asset Button
                Button {
                    id: btnUpload
                    text: "Upload File"
                    onClicked: {
                        if (typeof wsProjectPage !== "undefined") {
                            wsProjectPage.openUploadDialog()
                        }
                    }

                    contentItem: RowLayout {
                        spacing: 6
                        anchors.centerIn: parent
                        Text { text: "📤"; font.pixelSize: 11 }
                        Text {
                            text: "Upload"
                            color: "#E2E8F0"
                            font.pixelSize: 12
                            font.weight: Font.Medium
                            font.family: "Inter"
                        }
                    }

                    background: Rectangle {
                        implicitWidth: root.isCompact ? 96 : 106
                        implicitHeight: 36
                        radius: 8
                        color: btnUpload.down ? "#161824" : (btnUpload.hovered ? "#26293A" : "#1C1E2B")
                        border.color: btnUpload.hovered ? "#474D67" : "#2E3245"
                        border.width: 1
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }
                }
            }
        }
    }
}
