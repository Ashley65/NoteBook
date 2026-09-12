import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "wsNoteEditor" as WsNoteEditor


Item {
    id: root
    anchors.fill: parent

    property color bgDarker: "#14151F"
    property color bgDark: "#1B1D2B"
    property color textMain: "#FFFFFF"
    property color textMuted: "#94A3B8"
    property color accentBlue: "#60A5FA"
    property color accentPurple: "#8B5CF6"
    property color borderDark: "#2B2D3F"

    // UPGRADED: From boolean to string state management
    property string currentViewMode: "split" // "edit", "split", or "preview"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        WsNoteEditor.NoteHeader {
            id: noteHeader
            Layout.fillWidth: true
            onViewModeSelected: function(mode) {
                root.currentViewMode = mode
            }
            onFormatRequested: function(action) {
                if (editorPane) {
                    editorPane.applyFormatting(action);
                }
            }
        }

        SplitView {
            id: mainSplitView
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal
            spacing: 2

            handle: Rectangle {
                implicitWidth: 3
                color: SplitHandle.pressed ? "#8B5CF6" : (SplitHandle.hovered ? "#4C1D95" : "#1B1D2B")
            }

            // Editor Area
            WsNoteEditor.EditorArea {
                id: editorPane
                visible: root.currentViewMode !== "preview"
                SplitView.fillWidth: root.currentViewMode === "edit"
                SplitView.preferredWidth: root.currentViewMode === "split" ? parent.width * 0.5 : parent.width
            }

            // Preview Area
            WsNoteEditor.NotePreview {
                id: previewPane
                visible: root.currentViewMode !== "edit"
                SplitView.fillWidth: root.currentViewMode === "preview"
                SplitView.preferredWidth: root.currentViewMode === "split" ? parent.width * 0.5 : parent.width
            }
        }

        WsNoteEditor.NoteFooter {
            id: noteFooter
            Layout.fillWidth: true
        }
    }


    Binding {
        target: previewPane.verticalScrollBar
        property: "position"
        value: editorPane.verticalScrollBar.position
        when: editorPane.verticalScrollBar.active
    }

    Binding {
        target: editorPane.verticalScrollBar
        property: "position"
        value: previewPane.verticalScrollBar.position
        when: previewPane.verticalScrollBar.active
    }
}