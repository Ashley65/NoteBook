import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    color: root.bgDark
    border.color: root.borderDark
    border.width: 1

    property alias verticalScrollBar: previewScroll


    ScrollView {
        id: previewScroll
        anchors.fill: parent
        clip: true

        ScrollBar.vertical: ScrollBar { id: previewScrollBar }

        TextArea {
            width: parent.width
            padding: 20
            color: root.textMain
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
                // Eventually, you can call a C++ method here to open the linked note!
                notePageContext.onLinkClicked(link)
            }
        }
    }
}