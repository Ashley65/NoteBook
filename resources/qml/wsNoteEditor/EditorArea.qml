import QtQuick 2.15
import QtQuick.Controls 2.15
import "Popup" as Popup

Rectangle {
    color: root.bgDark
    property alias verticalScrollBar: editorScroll


    ScrollView {
        id: editorScroll
        anchors.fill: parent
        clip: true

        ScrollBar.vertical: ScrollBar { id: editorScrollBar }

        TextArea {
            id: textArea
            width: parent.width


            padding: 20
            color: root.textMain
            font.pixelSize: 15
            font.family: "Consolas"
            wrapMode: Text.WordWrap
            selectByMouse: true

            background: Rectangle { color: "transparent" }

            text: typeof notePageContext !== "undefined" && notePageContext ? notePageContext.currentNoteContent : ""

            onTextChanged: {
                if (typeof notePageContext === "undefined" || !notePageContext) return;

                // 1. Database Saving Logic
                if (notePageContext.currentNoteId === "-1" && text.trim() !== "") {
                    notePageContext.commitDraftToDatabase(text)
                } else if (notePageContext.currentNoteId !== "-1" && text !== notePageContext.currentNoteContent) {
                    notePageContext.updateExistingNote(notePageContext.currentNoteId, text)
                }

                // 2. Link Detection Logic
                var textBeforeCursor = text.substring(0, cursorPosition);
                var match = textBeforeCursor.match(/\[\[([^\]]*)$/);

                if (match) {
                    var searchTerm = match[1];
                    var results = notePageContext.searchNotesByTitle(searchTerm);

                    // Calculate cursor coordinates
                    var cursorRect = textArea.positionToRectangle(textArea.cursorPosition);
                    var px = cursorRect.x + textArea.padding;
                    var py = cursorRect.y + cursorRect.height + textArea.padding;

                    // Tell the external popup to show itself!
                    suggestionDropdown.updateAndOpen(px, py, results);
                } else {
                    suggestionDropdown.close();
                }
            }
        }
    }

    Popup.AutocompleteDropdown {
        id: suggestionDropdown

        onSuggestionSelected: function(title) {
            // Find where the "[[" started
            var textBeforeCursor = textArea.text.substring(0, textArea.cursorPosition);
            var bracketIndex = textBeforeCursor.lastIndexOf("[[");

            // Slice the text
            var stringStart = textArea.text.substring(0, bracketIndex);
            var stringEnd = textArea.text.substring(textArea.cursorPosition);

            // Insert the selected link
            textArea.text = stringStart + "[[" + title + "]]" + stringEnd;

            // Move cursor and refocus
            textArea.cursorPosition = bracketIndex + title.length + 4;
            textArea.forceActiveFocus();
        }
    }
}