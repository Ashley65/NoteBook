import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
//import "wsNotePage" as WsNotePage

Item {
    id: root
    anchors.fill: parent

    readonly property color bgDark: "#13141B"
    readonly property color surfaceDark: "#1C1E28"
    readonly property color surfaceHover: "#252836"
    readonly property color borderDark: Qt.rgba(1, 1, 1, 0.08)
    readonly property color textMain: "#F3F4F6"
    readonly property color textMuted: "#9CA3AF"
    readonly property color accentGreen: "#10B981"
    readonly property color accentBlue: "#3B82F6"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        // =========================================================
        // 1. HEADER & CONTROLS ROW
        // =========================================================
        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            // Title & Scope Badge
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
                    width: scopeRow.implicitWidth + 20
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
                    Layout.leftMargin: 20
                }
            }

            Item { Layout.fillWidth: true } // Spacer

            // Search Bar
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

                    Text {
                        text: "🔍"
                        font.pixelSize: 12
                        color: root.textMuted
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

            // Create New Note Button
            Rectangle {
                width: 140
                height: 34
                radius: 6
                color: newNoteMouse.containsMouse ? "#059669" : root.accentGreen

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

        // =========================================================
        // 2. FILTER PILLS ROW
        // =========================================================
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: [
                    { id: "all", label: "All Notes" },
                    { id: "pinned", label: "📌 Pinned" },
                    { id: "recent", label: "Recent" },
                    { id: "archived", label: "Archived" }
                ]

                delegate: Rectangle {
                    property bool isSelected: typeof noteListPage !== "undefined" && noteListPage.activeFilter === modelData.id
                    height: 28
                    width: filterLabel.implicitWidth + 20
                    radius: 14
                    color: isSelected ? "#2D3044" : (filterMouse.containsMouse ? root.surfaceHover : root.surfaceDark)
                    border.color: isSelected ? Qt.rgba(1, 1, 1, 0.2) : root.borderDark
                    border.width: 1

                    Text {
                        id: filterLabel
                        anchors.centerIn: parent
                        text: modelData.label
                        color: isSelected ? "#FFFFFF" : root.textMuted
                        font.pixelSize: 11
                        font.weight: isSelected ? Font.DemiBold : Font.Normal
                    }

                    MouseArea {
                        id: filterMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (typeof noteListPage !== "undefined") {
                                noteListPage.activeFilter = modelData.id
                            }
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }
        }

        // =========================================================
        // 3. MAIN CONTENT: CARDS GALLERY OR EMPTY STATE
        // =========================================================
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // --- EMPTY STATE ---
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 16
                visible: typeof noteListPage !== "undefined" && noteListPage.isEmpty

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 72
                    height: 72
                    radius: 36
                    color: Qt.rgba(1, 1, 1, 0.04)
                    border.color: root.borderDark
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "📝"
                        font.pixelSize: 32
                    }
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "No notes found"
                    font.pixelSize: 16
                    font.bold: true
                    color: root.textMain
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Create a note to start organizing project documentation."
                    font.pixelSize: 12
                    color: root.textMuted
                }

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 130
                    height: 32
                    radius: 6
                    color: root.accentGreen

                    Text {
                        anchors.centerIn: parent
                        text: "+ Create Note"
                        color: "#FFFFFF"
                        font.bold: true
                        font.pixelSize: 12
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (typeof noteListPage !== "undefined") {
                                noteListPage.createNewNote()
                            }
                        }
                    }
                }
            }

            // --- NOTES CARDS GRID ---
            GridView {
                id: notesGrid
                anchors.fill: parent
                clip: true
                cellWidth: Math.max(260, Math.floor(width / Math.max(1, Math.floor(width / 280))))
                cellHeight: 180
                visible: typeof noteListPage !== "undefined" && !noteListPage.isEmpty

                model: typeof noteListPage !== "undefined" ? noteListPage.notes : null

                delegate: Item {
                    width: notesGrid.cellWidth
                    height: notesGrid.cellHeight

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 6
                        radius: 8
                        color: cardMouse.containsMouse ? root.surfaceHover : root.surfaceDark
                        border.color: cardMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.18) : root.borderDark
                        border.width: 1

                        Behavior on color { ColorAnimation { duration: 100 } }
                        Behavior on border.color { ColorAnimation { duration: 100 } }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 8

                            // Card Top: Title + Pin Button
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                Text {
                                    text: modelData.title
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: root.textMain
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Text {
                                    text: modelData.isPinned ? "📌" : "☆"
                                    font.pixelSize: 13
                                    color: modelData.isPinned ? "#F59E0B" : root.textMuted
                                    visible: modelData.isPinned || cardMouse.containsMouse

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            if (typeof noteListPage !== "undefined") {
                                                noteListPage.togglePin(modelData.id)
                                            }
                                        }
                                    }
                                }

                                Text {
                                    text: "🗑"
                                    font.pixelSize: 12
                                    color: root.textMuted
                                    visible: cardMouse.containsMouse

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            if (typeof noteListPage !== "undefined") {
                                                noteListPage.deleteNote(modelData.id)
                                            }
                                        }
                                    }
                                }
                            }

                            // Card Body: Snippet Preview
                            Text {
                                text: modelData.preview !== "" ? modelData.preview : "No additional text"
                                font.pixelSize: 11
                                color: root.textMuted
                                wrapMode: Text.Wrap
                                elide: Text.ElideRight
                                maximumLineCount: 3
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                            }

                            // Card Footer: Metadata Pills
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                // Project Badge (if present)
                                Rectangle {
                                    visible: modelData.projectName !== ""
                                    height: 18
                                    width: projBadgeText.implicitWidth + 10
                                    radius: 4
                                    color: Qt.rgba(0.06, 0.72, 0.51, 0.12)

                                    Text {
                                        id: projBadgeText
                                        anchors.centerIn: parent
                                        text: modelData.projectName
                                        font.pixelSize: 9
                                        color: modelData.projectColor !== "" ? modelData.projectColor : root.accentGreen
                                        font.weight: Font.DemiBold
                                    }
                                }

                                Text {
                                    text: modelData.updatedAtFormatted
                                    font.pixelSize: 10
                                    color: root.textMuted
                                }

                                Item { Layout.fillWidth: true }

                                Text {
                                    text: modelData.wordCountFormatted
                                    font.pixelSize: 10
                                    color: root.textMuted
                                }
                            }
                        }

                        // Open Note on Click
                        MouseArea {
                            id: cardMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            z: -1
                            onClicked: {
                                if (typeof noteListPage !== "undefined") {
                                    noteListPage.openNote(modelData.id)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
