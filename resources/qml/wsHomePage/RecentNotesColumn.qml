import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "qrc:/qml/warningBox" as WarningBox

Rectangle {
    id: root
    color: "transparent"
    radius: 10
    border.color: "#3B4048"
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        Text {
            text: "Recent Notes"
            color: "#FFFFFF"
            font.pixelSize: 20
            font.bold: true
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8
            clip: true
            model: typeof wsHomePage !== "undefined" ? wsHomePage.recentNotes : 0

            delegate: Rectangle {
                id: noteCard
                width: ListView.view.width
                implicitHeight: 80
                radius: 8
                color: cardArea.containsMouse ? Qt.rgba(1, 1, 1, 0.15) : Qt.rgba(1, 1, 1, 0.05)
                border.color: "#3B4048"
                border.width: 1

                property string noteId: String(modelData.id || "")

                MouseArea {
                    id: cardArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    cursorShape: Qt.PointingHandCursor

                    onClicked: function(mouse) {
                        // ignore clicks if no id or wsHomePage is not available
                        if (typeof wsHomePage !== "undefined" && noteCard.noteId) {
                            console.log("Mock: Upload File Dialog Triggered")
                        }
                        mouse.accepted = true
                    }
                }

                Button {
                    id: btnNoteDelete
                    z: 2
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 8
                    width: 32
                    height: 32
                    icon.source: "qrc:/icons/delete-icon.svg"

                    opacity: (cardArea.containsMouse || btnNoteDelete.hovered) ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: 150 } }

                    background: Rectangle { color: "transparent" }

                    onClicked: {
                        // Use the delegate-local noteId so it's consistent
                        deleteDialog.noteIdToDelete = noteCard.noteId
                        deleteDialog.visible = true
                    }
                }



                Column {
                    anchors.left: parent.left
                    anchors.right: btnNoteDelete.left
                    anchors.top: parent.top
                    anchors.margins: 12
                    spacing: 6

                    Text {
                        text: modelData.title
                        color: "white"
                        font.bold: true
                        elide: Text.ElideRight
                        width: parent.width
                    }

                    Text {
                        id: excerptText
                        // NOTE: your C++ populates `preview` (not `excerpt`) — keep fields consistent.
                        text: (typeof modelData.preview === "string" && modelData.preview.trim().length > 0)
                            ? modelData.preview
                            : qsTr("No save data")
                        color: (typeof modelData.preview === "string" && modelData.preview.trim().length > 0)
                            ? "#A0A0A0"
                            : "#7F7F7F"
                        wrapMode: Text.Wrap
                        width: parent.width
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                }
            }
        }


        Button {
            id: btnViewAllNotes
            text: "View All Notes >"
            onClicked: console.log("Mock: View All Notes Clicked")

            Layout.alignment: Qt.AlignHCenter


            contentItem: Text {
                text: btnViewAllNotes.text
                color: "#E2E8F0"
                font.pixelSize: 14
                font.weight: Font.Medium
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                implicitWidth: 130
                implicitHeight: 40
                color: btnViewAllNotes.hovered ? Qt.rgba(1, 1, 1, 0.15) : Qt.rgba(1, 1, 1, 0.05)
                radius: 8
                border.color: "#3B4048"
                border.width: 1

                Behavior on color { ColorAnimation { duration: 150 } }
            }
        }
    }


    Popup {
        id: deleteDialog
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 500
        height: 200
        padding: 0


        Overlay.modal: Rectangle {
            color: Qt.rgba(0, 0, 0, 0.5)
        }

        property string noteIdToDelete: ""

        WarningBox.NoteDeletion {
            anchors.fill: parent
            onConfirmDelete: {
                if (typeof wsHomePage !== "undefined" && deleteDialog.noteIdToDelete) {
                    wsHomePage.deleteNote(String(deleteDialog.noteIdToDelete))
                }
                deleteDialog.visible = false
            }
            onCancel: {
                deleteDialog.visible = false
            }
        }
    }
}
