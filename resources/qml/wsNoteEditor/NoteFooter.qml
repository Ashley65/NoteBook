import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: footerRoot
    height: 32
    color: "#14151F"
    border.color: "#2B2D3F"
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 16

        // Left side: Document Statistics
        RowLayout {
            spacing: 12
            Layout.alignment: Qt.AlignVCenter

            Text {
                text: (typeof notePageContext !== "undefined" && notePageContext ? notePageContext.wordCount : 0) + " words"
                color: "#94A3B8"
                font.pixelSize: 11
                font.weight: Font.Medium
            }

            Text {
                text: "•"
                color: "#4B5563"
                font.pixelSize: 11
            }

            Text {
                text: (typeof notePageContext !== "undefined" && notePageContext ? notePageContext.characterCount : 0) + " characters"
                color: "#94A3B8"
                font.pixelSize: 11
            }

            Text {
                text: "•"
                color: "#4B5563"
                font.pixelSize: 11
            }

            Text {
                text: "~" + (typeof notePageContext !== "undefined" && notePageContext ? notePageContext.readingTimeMinutes : 0) + " min read"
                color: "#94A3B8"
                font.pixelSize: 11
            }
        }

        Item { Layout.fillWidth: true } // Spacer

        // Right side: Save Status & Engine Capabilities
        RowLayout {
            spacing: 10
            Layout.alignment: Qt.AlignVCenter

            // Status Indicator Dot
            Rectangle {
                width: 7
                height: 7
                radius: 3.5
                color: {
                    if (typeof notePageContext === "undefined" || !notePageContext) return "#10B981";
                    return notePageContext.saveStatus.indexOf("Unsaved") !== -1 ? "#F59E0B" : "#10B981";
                }
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                text: {
                    if (typeof notePageContext === "undefined" || !notePageContext) return "Saved";
                    return notePageContext.saveStatus.indexOf("Unsaved") !== -1 ? "Draft (Unsaved)" : "Saved";
                }
                color: {
                    if (typeof notePageContext === "undefined" || !notePageContext) return "#10B981";
                    return notePageContext.saveStatus.indexOf("Unsaved") !== -1 ? "#FBBF24" : "#10B981";
                }
                font.pixelSize: 11
                font.weight: Font.Medium
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                text: "|"
                color: "#374151"
                font.pixelSize: 11
            }

            Text {
                text: "Markdown & WikiLinks enabled"
                color: "#6B7280"
                font.pixelSize: 11
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }
}