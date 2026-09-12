import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "Popup" as Popup

Rectangle {
    id: editorContainer
    color: "#14151F"
    border.color: dropArea.containsDrag ? "#8B5CF6" : "#2B2D3F"
    border.width: dropArea.containsDrag ? 2 : 1
    radius: 0

    property alias verticalScrollBar: editorScrollBar

    function applyFormatting(action) {
        var start = textArea.selectionStart;
        var end = textArea.selectionEnd;
        var hasSelection = (start !== end);
        var selectedText = hasSelection ? textArea.text.substring(start, end) : "";

        if (action === "bold") {
            var replacement = "**" + (hasSelection ? selectedText : "bold text") + "**";
            insertOrWrap(replacement, start, end, 2, replacement.length - 2);
        } else if (action === "italic") {
            var replacement = "*" + (hasSelection ? selectedText : "italic text") + "*";
            insertOrWrap(replacement, start, end, 1, replacement.length - 1);
        } else if (action === "heading") {
            insertLinePrefix("## ");
        } else if (action === "code") {
            if (hasSelection && selectedText.indexOf("\n") !== -1) {
                var replacement = "```\n" + selectedText + "\n```\n";
                insertOrWrap(replacement, start, end, 4, replacement.length - 4);
            } else {
                var replacement = "`" + (hasSelection ? selectedText : "code") + "`";
                insertOrWrap(replacement, start, end, 1, replacement.length - 1);
            }
        } else if (action === "list") {
            insertLinePrefix("- ");
        } else if (action === "task") {
            insertLinePrefix("- [ ] ");
        } else if (action === "table") {
            var tableSnippet = "\n| Column 1 | Column 2 | Column 3 |\n| -------- | -------- | -------- |\n| Item 1   | Value A   | 100      |\n| Item 2   | Value B   | 200      |\n";
            insertSnippetAtCursor(tableSnippet);
        } else if (action === "quote") {
            insertLinePrefix("> ");
        }
    }

    function insertOrWrap(replacement, start, end, innerStartOffset, innerEndOffset) {
        var fullText = textArea.text;
        var before = fullText.substring(0, start);
        var after = fullText.substring(end);
        textArea.text = before + replacement + after;
        textArea.select(start + innerStartOffset, start + innerEndOffset);
        textArea.forceActiveFocus();
    }

    function insertLinePrefix(prefix) {
        var pos = textArea.cursorPosition;
        var fullText = textArea.text;
        var lastNewline = fullText.lastIndexOf("\n", pos - 1);
        var lineStart = (lastNewline === -1) ? 0 : lastNewline + 1;

        var before = fullText.substring(0, lineStart);
        var after = fullText.substring(lineStart);
        textArea.text = before + prefix + after;
        textArea.cursorPosition = pos + prefix.length;
        textArea.forceActiveFocus();
    }

    function insertSnippetAtCursor(snippet) {
        var pos = textArea.cursorPosition;
        var fullText = textArea.text;
        var before = fullText.substring(0, pos);
        var after = fullText.substring(pos);
        textArea.text = before + snippet + after;
        textArea.cursorPosition = pos + snippet.length;
        textArea.forceActiveFocus();
    }

    // Drop Area for File Attachments
    DropArea {
        id: dropArea
        anchors.fill: parent
        onDropped: {
            if (drop.hasUrls && typeof notePageContext !== "undefined" && notePageContext) {
                var tagsToInsert = "";
                for (var i = 0; i < drop.urls.length; i++) {
                    var tag = notePageContext.attachFileToCurrentNote(drop.urls[i].toString());
                    if (tag.length > 0) {
                        tagsToInsert += tag;
                    }
                }
                if (tagsToInsert.length > 0) {
                    insertSnippetAtCursor(tagsToInsert);
                }
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Gutter: Line Numbers
        Rectangle {
            id: gutter
            Layout.preferredWidth: 44
            Layout.fillHeight: true
            color: "#11121A"
            border.color: "#232536"
            border.width: 1

            Flickable {
                id: gutterFlickable
                anchors.fill: parent
                clip: true
                contentY: textArea.cursorRectangle.y // rough sync or binding
                interactive: false

                Column {
                    anchors.top: parent.top
                    anchors.topMargin: 20 // match textArea top padding
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    spacing: 0

                    Repeater {
                        model: Math.max(1, textArea.lineCount)
                        Text {
                            text: (index + 1).toString()
                            font.family: "Consolas, monospace"
                            font.pixelSize: 13
                            color: "#4B5563"
                            height: 22 // line height approximation
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }
        }

        // Main Editor Scroll Area
        ScrollView {
            id: editorScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ScrollBar.vertical: ScrollBar {
                id: editorScrollBar
                policy: ScrollBar.AsNeeded
            }

            TextArea {
                id: textArea
                width: editorScroll.width
                padding: 20
                color: "#F3F4F6"
                font.pixelSize: 14
                font.family: "Consolas, 'Cascadia Code', monospace"
                wrapMode: Text.WordWrap
                selectByMouse: true

                background: Rectangle { color: "transparent" }

                text: typeof notePageContext !== "undefined" && notePageContext ? notePageContext.currentNoteContent : ""

                onTextChanged: {
                    if (typeof notePageContext === "undefined" || !notePageContext) return;

                    // 1. Database Saving Logic
                    if (notePageContext.currentNoteId === "-1" && text.trim() !== "") {
                        notePageContext.commitDraftToDatabase(text);
                    } else if (notePageContext.currentNoteId !== "-1" && text !== notePageContext.currentNoteContent) {
                        notePageContext.updateExistingNote(notePageContext.currentNoteId, text);
                    }

                    // 2. Attachment Embed & Link Detection Logic
                    var textBeforeCursor = text.substring(0, cursorPosition);

                    // Check for attachment embed ![[
                    var attachMatch = textBeforeCursor.match(/!\[\[([^\]]*)$/);
                    if (attachMatch) {
                        var attachQuery = attachMatch[1];
                        var attachResults = notePageContext.searchAttachments(attachQuery);
                        suggestionDropdown.isAttachmentMode = true;

                        var cursorRect = textArea.positionToRectangle(textArea.cursorPosition);
                        var px = cursorRect.x + textArea.padding + gutter.width;
                        var py = cursorRect.y + cursorRect.height + textArea.padding;

                        suggestionDropdown.updateAndOpen(px, py, attachResults);
                        return;
                    }

                    // Check for note wiki-link [[
                    var match = textBeforeCursor.match(/\[\[([^\]]*)$/);
                    if (match) {
                        suggestionDropdown.isAttachmentMode = false;
                        var searchTerm = match[1];
                        var results = notePageContext.searchNotesByTitle(searchTerm);

                        var cursorRect = textArea.positionToRectangle(textArea.cursorPosition);
                        var px = cursorRect.x + textArea.padding + gutter.width;
                        var py = cursorRect.y + cursorRect.height + textArea.padding;

                        suggestionDropdown.updateAndOpen(px, py, results);
                    } else {
                        suggestionDropdown.close();
                    }
                }
            }
        }
    }

    // Autocomplete Dropdown
    Popup.AutocompleteDropdown {
        id: suggestionDropdown
        property bool isAttachmentMode: false

        onSuggestionSelected: function(title) {
            var prefix = isAttachmentMode ? "![[" : "[[";
            var textBeforeCursor = textArea.text.substring(0, textArea.cursorPosition);
            var bracketIndex = textBeforeCursor.lastIndexOf(prefix);
            if (bracketIndex < 0) {
                bracketIndex = textBeforeCursor.lastIndexOf("[[");
                prefix = "[[";
            }

            var stringStart = textArea.text.substring(0, bracketIndex);
            var stringEnd = textArea.text.substring(textArea.cursorPosition);

            textArea.text = stringStart + prefix + title + "]]" + stringEnd;

            textArea.cursorPosition = bracketIndex + prefix.length + title.length + 2;
            textArea.forceActiveFocus();
        }
    }
}