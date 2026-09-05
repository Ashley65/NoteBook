import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    Layout.fillWidth: true
    implicitHeight: mainHeaderLayout.implicitHeight

    readonly property color bgDark: "#13141B"
    readonly property color surfaceDark: "#1C1E28"
    readonly property color surfaceHover: "#252836"
    readonly property color borderDark: Qt.rgba(1, 1, 1, 0.08)
    readonly property color textMain: "#F3F4F6"
    readonly property color textMuted: "#9CA3AF"
    readonly property color accentGreen: "#10B981"
    readonly property color accentBlue: "#3B82F6"

    RowLayout {
        id: mainHeaderLayout
        anchors.fill: parent
        spacing: 16

        // 1. Title + Project Scope Badge + Note Count
        RowLayout {
            spacing: 10

            Text {
                text: "Notes"
                font.pixelSize: 22
                font.bold: true
                color: root.textMain
                font.family: "Segoe UI, Inter, sans-serif"
            }

            Rectangle {
                visible: typeof noteListPage !== "undefined" && noteListPage.isProjectScoped
                height: 24
                width: scopeRow.implicitWidth + 16
                radius: 12
                color: Qt.rgba(0.06, 0.72, 0.51, 0.15)
                border.color: Qt.rgba(0.06, 0.72, 0.51, 0.35)
                border.width: 1

                RowLayout {
                    id: scopeRow
                    anchors.centerIn: parent
                    spacing: 6

                    Rectangle {
                        width: 6
                        height: 6
                        radius: 3
                        color: typeof noteListPage !== "undefined" ? noteListPage.projectColor : root.accentGreen
                    }

                    Text {
                        text: typeof noteListPage !== "undefined" ? noteListPage.projectName : ""
                        color: root.accentGreen
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }
                }
            }

            Text {
                text: typeof noteListPage !== "undefined" ? "(" + noteListPage.totalNoteCount + ")" : ""
                color: root.textMuted
                font.pixelSize: 14
                Layout.leftMargin: 10
            }
        }

        // 2. Spacer pushes Search Bar and Button to the right
        Item { Layout.fillWidth: true }

        // 3. Search Bar
        Rectangle {
            width: 240
            height: 34
            radius: 6
            color: root.surfaceDark
            border.color: searchInput.activeFocus ? root.accentBlue : root.borderDark
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 8
                spacing: 6

                Image{
                    source: "qrc:/icons/search.svg"
                    width: 14
                    height: 14
                    fillMode: Image.PreserveAspectFit
                }

                TextInput {
                    id: searchInput
                    Layout.fillWidth: true
                    color: root.textMain
                    font.pixelSize: 12
                    selectByMouse: true
                    clip: true

                    Text {
                        text: "Search notes..."
                        color: root.textMuted
                        visible: !searchInput.text && !searchInput.activeFocus
                        font.pixelSize: 12
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    onTextChanged: {
                        if (typeof noteListPage !== "undefined") {
                            noteListPage.searchQuery = text
                        }
                    }
                }

                Text {
                    text: "✕"
                    color: root.textMuted
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

        // 4. Create New Note Button
        Rectangle {
            width: 110
            height: 34
            radius: 6
            color: newNoteMouse.containsMouse ? "#059669" : root.accentGreen
            border.color: newNoteMouse.containsMouse ? "#7E7E7E" : root.borderDark
            border.width: 1

            Behavior on color { ColorAnimation { duration: 100 } }

            RowLayout {
                anchors.centerIn: parent
                spacing: 6

                Text {
                    text: "+"
                    color: "#FFFFFF"
                    font.pixelSize: 16
                    font.bold: true
                }

                Text {
                    text: "New Note"
                    color: "#FFFFFF"
                    font.pixelSize: 12
                    font.bold: true
                }
            }

            MouseArea {
                id: newNoteMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (typeof noteListPage !== "undefined") {
                        noteListPage.createNewNote()
                    }
                }
            }
        }
    }
}