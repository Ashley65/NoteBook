import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "Button" as ToggleButtons

Rectangle {
    id: headerRoot
    height: 50

    // 1. Define the colors locally to fix the Scope issue!
    property color bgDarker: "#181818"
    property color borderDark: "#2D2D2D"
    property color textMain: "#D4D4D4"
    property color textMuted: "#858585"
    property color accentPurple: "#C586C0"

    // 2. Use the local properties instead of the undefined 'root'
    color: headerRoot.bgDarker
    border.color: headerRoot.borderDark
    border.width: 1

    // Signal caught by the Linker to hide/show the preview pane
    signal togglePreviewClicked()
    signal viewModeSelected(string mode)

    RowLayout {
        anchors.fill: parent
        anchors.margins: 15

        TextField {
            text: notePageContext.currentNoteTitle !== "" ? notePageContext.currentNoteTitle : "Untitled Note"
            color: headerRoot.textMain
            font.bold: true
            font.pixelSize: 16
            Layout.fillWidth: true
            background: Rectangle { color: "transparent" }


            onEditingFinished: {
                if (text !== notePageContext.currentNoteTitle) {

                    notePageContext.updateNoteTitle(notePageContext.currentNoteId, text)
                }
            }
        }

        Label {
            text: notePageContext.saveStatus
            color: notePageContext.currentNoteId === "-1" ? headerRoot.accentPurple : headerRoot.textMuted
            font.pixelSize: 12
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: 15
        }

        ToggleButtons.ToggleButtons {
            Layout.alignment: Qt.AlignHCenter
            onViewModeChanged: function(mode) {
                headerRoot.viewModeSelected(mode);
            }
        }
    }
}