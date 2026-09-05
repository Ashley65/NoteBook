import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    Layout.fillWidth: true
    implicitHeight: fillterPillsLayout.implicitHeight

    readonly property color bgDark: "#13141B"
    readonly property color surfaceDark: "#1C1E28"
    readonly property color surfaceHover: "#252836"
    readonly property color borderDark: Qt.rgba(1, 1, 1, 0.08)
    readonly property color textMain: "#F3F4F6"
    readonly property color textMuted: "#9CA3AF"
    readonly property color accentGreen: "#10B981"
    readonly property color accentBlue: "#3B82F6"


    RowLayout{
        id: fillterPillsLayout
        Layout.fillWidth: true
        spacing: 8

        Repeater{
            model: [
                { id: "all", label: "All Notes" },
                { id: "pinned", label: "⭐ Pinned" },
                { id: "recent", label: "Recent" },
                { id: "archived", label: "Archived" }
            ]

            delegate: Rectangle {
                property bool isSelected: typeof noteListPage !== "undefined" && noteListPage.activeFilter === modelData.id
                height: 28
                width: filterLabel.implicitWidth + 20
                radius: 14
                color: isSelected ? "#2D3044" : (filterMouse.containsMouse ? root.surfaceHover : root.surfaceDark)
                border.color: isSelected ? Qt.rgba(1, 1, 1, 0.2) : root.borderDark
                border.width: 1

                Text {
                    id: filterLabel
                    anchors.centerIn: parent
                    text: modelData.label
                    color: isSelected ? "#FFFFFF" : root.textMuted
                    font.pixelSize: 11
                    font.weight: isSelected ? Font.DemiBold : Font.Normal
                }

                MouseArea {
                    id: filterMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (typeof noteListPage !== "undefined") {
                            noteListPage.activeFilter = modelData.id
                        }
                    }
                }
            }


        }
        Item { Layout.fillWidth: true }
    }
}