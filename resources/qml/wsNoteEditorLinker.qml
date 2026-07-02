import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "wsNoteEditor" as WsNoteEditor


Item {
    id: root
    anchors.fill: parent

    property color bgDarker: "#181818"
    property color bgDark: "#1E1E1E"
    property color textMain: "#D4D4D4"
    property color textMuted: "#858585"
    property color accentBlue: "#569CD6"
    property color accentPurple: "#C586C0"
    property color borderDark: "#2D2D2D"

    // UPGRADED: From boolean to string state management
    property string currentViewMode: "split" // "edit", "split", or "preview"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 1
        spacing: 2




        WsNoteEditor.NoteHeader {
            Layout.fillWidth: true
            // Listen for the new string-based signal
            onViewModeSelected: function(mode) {
                root.currentViewMode = mode
            }
        }

        SplitView {
            id: mainSplitView
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal
            spacing: 6

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