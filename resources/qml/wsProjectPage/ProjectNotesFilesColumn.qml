import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../warningBox" as WarningBox

Rectangle {
    id: root
    color: "#161824"
    radius: 12
    border.color: Qt.rgba(1, 1, 1, 0.08)
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // =========================================================
        // 1. PROJECT DOCUMENTATION & NOTES
        // =========================================================
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 280
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "Project Documentation"
                    color: "#FFFFFF"
                    font.pixelSize: 15
                    font.bold: true
                    font.family: "Inter"
                }
                Item { Layout.fillWidth: true }
                Rectangle {
                    height: 20
                    width: notesCountText.implicitWidth + 12
                    radius: 10
                    color: Qt.rgba(1, 1, 1, 0.06)
                    Text {
                        id: notesCountText
                        anchors.centerIn: parent
                        text: (typeof wsProjectPage !== "undefined" ? wsProjectPage.notes.length : 0) + " notes"
                        color: "#94A3B8"
                        font.pixelSize: 10
                        font.family: "Inter"
                    }
                }
            }

            // Empty State Notes
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: typeof wsProjectPage !== "undefined" && wsProjectPage.notes.length === 0

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8
                    Text { Layout.alignment: Qt.AlignHCenter; text: "📝"; font.pixelSize: 24 }
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "No project notes yet"
                        color: "#94A3B8"
                        font.pixelSize: 12
                    }
                }
            }

            // Notes Grid
            GridView {
                id: projNotesGrid
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                visible: typeof wsProjectPage !== "undefined" && wsProjectPage.notes.length > 0

                readonly property bool isTwoCol: projNotesGrid.width >= 440
                cellWidth: isTwoCol ? Math.floor(projNotesGrid.width / 2) : projNotesGrid.width
                cellHeight: isTwoCol ? 124 : 96
                model: typeof wsProjectPage !== "undefined" ? wsProjectPage.notes : []

                delegate: Item {
                    width: projNotesGrid.cellWidth
                    height: projNotesGrid.cellHeight

                    Rectangle {
                        id: noteCardRect
                        anchors.fill: parent
                        anchors.margins: 4
                        radius: 8
                        color: cardMouse.containsMouse ? "#232637" : "#1B1D2B"
                        border.color: cardMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.16) : Qt.rgba(1, 1, 1, 0.08)
                        border.width: 1

                        property string noteId: String(modelData.id || "")

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 4

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6
                                Text {
                                    text: modelData.title || "Untitled Note"
                                    font.pixelSize: 12
                                    font.bold: true
                                    color: "#FFFFFF"
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                    font.family: "Inter"
                                }
                                Text {
                                    text: modelData.isPinned ? "📌" : ""
                                    font.pixelSize: 10
                                    visible: modelData.isPinned
                                }
                                Text {
                                    text: "✕"
                                    font.pixelSize: 10
                                    color: "#94A3B8"
                                    visible: cardMouse.containsMouse
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            if (typeof wsProjectPage !== "undefined" && noteCardRect.noteId) {
                                                wsProjectPage.deleteNote(noteCardRect.noteId)
                                            }
                                        }
                                    }
                                }
                            }

                            Text {
                                text: (typeof modelData.preview === "string" && modelData.preview.trim().length > 0)
                                    ? modelData.preview
                                    : "No preview text"
                                font.pixelSize: 11
                                color: "#94A3B8"
                                wrapMode: Text.Wrap
                                elide: Text.ElideRight
                                maximumLineCount: 2
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                font.family: "Inter"
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6
                                Text {
                                    text: modelData.updatedAtFormatted || ""
                                    font.pixelSize: 10
                                    color: "#64748B"
                                    font.family: "Inter"
                                }
                                Item { Layout.fillWidth: true }
                                Text {
                                    text: modelData.wordCountFormatted || ""
                                    font.pixelSize: 10
                                    color: "#64748B"
                                    font.family: "Inter"
                                }
                            }
                        }

                        MouseArea {
                            id: cardMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            z: -1
                            onClicked: {
                                if (typeof wsProjectPage !== "undefined" && noteCardRect.noteId) {
                                    wsProjectPage.openNote(noteCardRect.noteId)
                                }
                            }
                        }
                    }
                }
            }
        }

        // Divider
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Qt.rgba(1, 1, 1, 0.06)
        }

        // =========================================================
        // 2. ATTACHED ASSETS & FILES
        // =========================================================
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 180
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "Attached Assets & Files"
                    color: "#FFFFFF"
                    font.pixelSize: 15
                    font.bold: true
                    font.family: "Inter"
                }
                Item { Layout.fillWidth: true }
                Rectangle {
                    height: 20
                    width: filesCountText.implicitWidth + 12
                    radius: 10
                    color: Qt.rgba(1, 1, 1, 0.06)
                    Text {
                        id: filesCountText
                        anchors.centerIn: parent
                        text: (typeof wsProjectPage !== "undefined" ? wsProjectPage.attachments.length : 0) + " files"
                        color: "#94A3B8"
                        font.pixelSize: 10
                        font.family: "Inter"
                    }
                }
            }

            // Empty State Files
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: typeof wsProjectPage !== "undefined" && wsProjectPage.attachments.length === 0

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 8
                    Text { Layout.alignment: Qt.AlignHCenter; text: "📁"; font.pixelSize: 24 }
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: "No files attached to this project"
                        color: "#94A3B8"
                        font.pixelSize: 12
                    }
                }
            }

            // Files List
            ListView {
                id: filesListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 6
                clip: true
                visible: typeof wsProjectPage !== "undefined" && wsProjectPage.attachments.length > 0
                model: typeof wsProjectPage !== "undefined" ? wsProjectPage.attachments : []

                delegate: Rectangle {
                    id: fileItemRect
                    width: filesListView.width
                    height: 42
                    radius: 6
                    color: fileMouse.containsMouse ? "#232637" : "#1B1D2B"
                    border.color: fileMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.16) : Qt.rgba(1, 1, 1, 0.06)
                    border.width: 1

                    property string fileId: String(modelData.id || "")

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        Text {
                            text: modelData.iconEmoji || "📄"
                            font.pixelSize: 14
                        }

                        Text {
                            text: modelData.fileName || "File"
                            color: "#FFFFFF"
                            font.pixelSize: 12
                            font.family: "Inter"
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Text {
                            text: modelData.fileSizeFormatted || ""
                            color: "#64748B"
                            font.pixelSize: 10
                            font.family: "Inter"
                        }

                        Text {
                            text: "✕"
                            font.pixelSize: 11
                            color: "#94A3B8"
                            visible: fileMouse.containsMouse
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (typeof wsProjectPage !== "undefined" && fileItemRect.fileId) {
                                        wsProjectPage.deleteAttachment(fileItemRect.fileId)
                                    }
                                }
                            }
                        }
                    }

                    MouseArea {
                        id: fileMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        z: -1
                        onClicked: {
                            if (typeof wsProjectPage !== "undefined" && fileItemRect.fileId) {
                                wsProjectPage.openAttachment(fileItemRect.fileId)
                            }
                        }
                    }
                }
            }
        }
    }
}
