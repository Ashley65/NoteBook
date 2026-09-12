import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: previewContainer
    color: "#14151F"
    border.color: "#2B2D3F"
    border.width: 1

    property alias verticalScrollBar: previewScrollBar

    ScrollView {
        id: previewScroll
        anchors.fill: parent
        anchors.margins: 1
        clip: true

        ScrollBar.vertical: ScrollBar {
            id: previewScrollBar
            policy: ScrollBar.AsNeeded
        }

        TextArea {
            id: previewText
            width: previewScroll.width
            padding: 28
            color: "#E2E8F0"
            font.pixelSize: 15

            wrapMode: TextEdit.Wrap
            textFormat: TextEdit.RichText

            readOnly: true
            selectByMouse: true

            background: Rectangle { color: "transparent" }

            text: typeof notePageContext !== "undefined" && notePageContext ?
                notePageContext.renderMarkdownToHtml(notePageContext.currentNoteContent) : ""

            onLinkActivated: function(link) {
                console.log("WikiLink clicked: " + link)
                notePageContext.onLinkClicked(link)
            }
        }
    }
}