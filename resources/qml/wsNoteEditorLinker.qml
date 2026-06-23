import QtQuick 2.15
import QtQuick.Layouts 1.15
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

    property bool isPreviewVisible: true

    ColumnLayoutn{
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16


        WsNoteEditor.NoteHeader {
            Layout.fillWidth: true
        }

        // The main content area is split into two parts: the editor and the preview
        // The SplitView allows the user to resize the two areas by dragging the divider while automatically adjust the sizes of the two areas when the window is resized
        // Also the button to toggle the preview in the header will hide the preview area and make the editor area take the full width, and vice versa
        SplitView {
            id: mainSplitView
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal
            spacing: 8

            // Editor Area
            WsNoteEditor.EditorArea {
                SplitView.fillWidth: true
                SplitView.preferredWidth: parent.width * (root.isPreviewVisible ? 0.5 : 1.0)
            }

            // Preview Area
            WsNoteEditor.NotePreview {
                visible: root.isPreviewVisible
                SplitView.preferredWidth: parent.width * 0.5
            }
        }

        WsNoteEditor.NoteFooter {
            Layout.fillWidth: true
        }
    }
}