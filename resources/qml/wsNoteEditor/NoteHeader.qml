import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "Button" as ToggleButtons

Rectangle {
    id: headerRoot
    height: 136

    property color bgDarker: "#141520"
    property color borderDark: "#2B2D3F"
    property color textMain: "#FFFFFF"
    property color textMuted: "#9CA3AF"
    property color accentPurple: "#8B5CF6"

    color: headerRoot.bgDarker
    border.color: headerRoot.borderDark
    border.width: 1

    signal togglePreviewClicked()
    signal viewModeSelected(string mode)
    signal formatRequested(string action)

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        anchors.topMargin: 12
        anchors.bottomMargin: 12
        spacing: 8

        // ROW 1: Breadcrumb and Live Metadata
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            // Project Color Indicator Dot
            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: typeof notePageContext !== "undefined" && notePageContext ? notePageContext.projectColor : "#8B5CF6"
                Layout.alignment: Qt.AlignVCenter
            }

            // Breadcrumbs: Workspace / Project
            Text {
                text: {
                    var ws = (typeof notePageContext !== "undefined" && notePageContext) ? notePageContext.workspaceName : "Workspace";
                    var proj = (typeof notePageContext !== "undefined" && notePageContext) ? notePageContext.projectName : "General";
                    return ws + "  /  " + proj;
                }
                color: "#94A3B8"
                font.pixelSize: 12
                font.weight: Font.Medium
                Layout.alignment: Qt.AlignVCenter
            }

            Item { Layout.fillWidth: true } // Spacer

            // Live Metadata Stats
            Text {
                text: {
                    if (typeof notePageContext === "undefined" || !notePageContext) return "";
                    var words = notePageContext.wordCount;
                    var readTime = notePageContext.readingTimeMinutes;
                    var status = notePageContext.saveStatus;
                    return status + "  •  " + words + " words  •  ~" + readTime + " min read";
                }
                color: "#8C92A4"
                font.pixelSize: 12
                Layout.alignment: Qt.AlignVCenter
            }
        }

        // ROW 2: Document Title (Full Width, Prominent)
        TextField {
            id: titleField
            text: typeof notePageContext !== "undefined" && notePageContext.currentNoteTitle !== ""
                  ? notePageContext.currentNoteTitle : ""
            placeholderText: "Untitled Note..."
            placeholderTextColor: "#64748B"
            color: headerRoot.textMain
            font.bold: true
            font.pixelSize: 22
            Layout.fillWidth: true
            background: Rectangle {
                color: "transparent"
                border.color: titleField.activeFocus ? "#8B5CF6" : "transparent"
                border.width: 1
                radius: 4
            }
            padding: 2

            onEditingFinished: {
                if (typeof notePageContext !== "undefined" && notePageContext && text !== notePageContext.currentNoteTitle) {
                    notePageContext.updateNoteTitle(notePageContext.currentNoteId, text);
                }
            }
        }

        // ROW 3: Dedicated Toolbar Row (Formatting + Attach on Left, View Mode Switcher on Right)
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            // Left Section: Formatting Toolbar Container Pill
            Rectangle {
                implicitHeight: 34
                implicitWidth: toolbarRow.implicitWidth + 8
                color: "#181926"
                border.color: "#2B2D3F"
                border.width: 1
                radius: 6

                Row {
                    id: toolbarRow
                    anchors.centerIn: parent
                    spacing: 2

                    // Helper component for formatting buttons
                    Component {
                        id: formatBtnComponent
                        ToolButton {
                            id: fBtn
                            property string actionName: ""
                            property string iconSource: ""
                            property string toolTipText: ""

                            implicitWidth: 30
                            implicitHeight: 28
                            padding: 0

                            contentItem: Item {
                                anchors.fill: parent
                                Image {
                                    anchors.centerIn: parent
                                    source: fBtn.iconSource
                                    width: 16
                                    height: 16
                                    fillMode: Image.PreserveAspectFit
                                    opacity: fBtn.hovered ? 1.0 : 0.65
                                }
                            }

                            background: Rectangle {
                                color: fBtn.down ? "#3B3F58" : (fBtn.hovered ? "#2A2D40" : "transparent")
                                radius: 4
                            }

                            ToolTip.visible: hovered
                            ToolTip.text: toolTipText
                            ToolTip.delay: 300

                            onClicked: headerRoot.formatRequested(actionName)
                        }
                    }

                    Loader {
                        sourceComponent: formatBtnComponent
                        onLoaded: { item.actionName = "bold"; item.iconSource = "qrc:/icons/format_bold.svg"; item.toolTipText = "Bold (Ctrl+B)"; }
                    }
                    Loader {
                        sourceComponent: formatBtnComponent
                        onLoaded: { item.actionName = "italic"; item.iconSource = "qrc:/icons/format_italic.svg"; item.toolTipText = "Italic (Ctrl+I)"; }
                    }
                    Loader {
                        sourceComponent: formatBtnComponent
                        onLoaded: { item.actionName = "heading"; item.iconSource = "qrc:/icons/format_heading.svg"; item.toolTipText = "Heading"; }
                    }
                    Loader {
                        sourceComponent: formatBtnComponent
                        onLoaded: { item.actionName = "code"; item.iconSource = "qrc:/icons/format_code.svg"; item.toolTipText = "Code Block"; }
                    }

                    // Divider inside toolbar
                    Rectangle {
                        width: 1
                        height: 16
                        color: "#2B2D3F"
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Loader {
                        sourceComponent: formatBtnComponent
                        onLoaded: { item.actionName = "list"; item.iconSource = "qrc:/icons/format_list_bulleted.svg"; item.toolTipText = "Bulleted List"; }
                    }
                    Loader {
                        sourceComponent: formatBtnComponent
                        onLoaded: { item.actionName = "task"; item.iconSource = "qrc:/icons/format_list_checkbox.svg"; item.toolTipText = "Task Checklist"; }
                    }
                    Loader {
                        sourceComponent: formatBtnComponent
                        onLoaded: { item.actionName = "table"; item.iconSource = "qrc:/icons/format_table.svg"; item.toolTipText = "Insert Table"; }
                    }
                    Loader {
                        sourceComponent: formatBtnComponent
                        onLoaded: { item.actionName = "quote"; item.iconSource = "qrc:/icons/format_quote.svg"; item.toolTipText = "Blockquote"; }
                    }
                }
            }

            // Attach File Button
            Button {
                id: btnAttach
                text: "📎 Attach"
                font.pixelSize: 12
                font.weight: Font.DemiBold
                Layout.alignment: Qt.AlignVCenter

                contentItem: Text {
                    text: btnAttach.text
                    font: btnAttach.font
                    color: btnAttach.hovered ? "#FFFFFF" : "#C4B5FD"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    implicitWidth: 80
                    implicitHeight: 32
                    color: btnAttach.down ? "#5B21B6" : (btnAttach.hovered ? "#4C1D95" : "#221338")
                    border.color: btnAttach.hovered ? "#8B5CF6" : "#6D28D9"
                    border.width: 1
                    radius: 6
                }

                onClicked: {
                    if (typeof notePageContext !== "undefined" && notePageContext) {
                        notePageContext.openAttachmentDialog();
                    }
                }
            }

            Item { Layout.fillWidth: true } // Spacer

            // View Mode Switcher Pill
            ToggleButtons.ToggleButtons {
                Layout.alignment: Qt.AlignVCenter
                onViewModeChanged: function(mode) {
                    headerRoot.viewModeSelected(mode);
                }
            }
        }
    }
}