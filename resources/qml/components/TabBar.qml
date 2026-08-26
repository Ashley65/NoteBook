import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls

// ---------------------------------------------------------
// Unified Tab Strip Track (Matching Top Menu Bar Aesthetic)
// ---------------------------------------------------------
Rectangle {
    id: topBarContainer
    property bool darkmode: true

    // Dedicated Container Track matching top menu bar
    color: "#16171E"
    radius: 0
    border.color: Qt.rgba(1, 1, 1, 0.08)
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 4
        anchors.topMargin: 2
        anchors.bottomMargin: 2
        spacing: 6

        // ---------------------------------------------------------
        // Dynamic Horizontal Tab Strip
        // ---------------------------------------------------------
        ListView {
            id: tabListView
            Layout.fillWidth: true
            Layout.fillHeight: true

            orientation: ListView.Horizontal
            spacing: 4
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            // Connect to C++ TabManager
            model: typeof tabManager !== "undefined" ? tabManager.tabs : null

            // Smooth animations for tabs
            add: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 120 } }
            remove: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 120 } }
            displaced: Transition { NumberAnimation { properties: "x,y"; duration: 120 } }

            // --- TAB ITEM DELEGATE ---
            delegate: Item {
                id: tabDelegate
                height: tabListView.height
                width: Math.min(240, Math.max(80, contentRow.implicitWidth + 20))

                property bool isActive: typeof tabManager !== "undefined" && tabManager.activeTabId === modelData.contextId
                property bool isHovered: hoverArea.containsMouse

                // Tab Click Selection Area (Background layer)
                MouseArea {
                    id: hoverArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (typeof tabManager !== "undefined") {
                            tabManager.activeTabId = modelData.contextId;
                        }
                    }
                }

                // Tab Surface (Foreground layer)
                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height
                    radius: 5

                    // Background matching top menu bar / sidebar styling
                    color: {
                        if (isActive) return "#2A2D3D";
                        if (isHovered) return "#20222E";
                        return "transparent";
                    }

                    // Border styling
                    property color borderColor: {
                        if (isActive) return Qt.rgba(1, 1, 1, 0.16);
                        if (isHovered) return Qt.rgba(1, 1, 1, 0.09);
                        return "transparent";
                    }
                    border.color: borderColor
                    border.width: 1

                    Behavior on color { ColorAnimation { duration: 100 } }
                    Behavior on borderColor { ColorAnimation { duration: 100 } }

                    // Tab Content: Dot + Title + Close Button
                    RowLayout {
                        id: contentRow
                        anchors.centerIn: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 6
                        spacing: 6

                        // Circular Color Badge
                        Rectangle {
                            width: 8
                            height: 8
                            radius: 4
                            color: modelData.projectColour !== undefined ? modelData.projectColour : "#3B82F6"
                            Layout.alignment: Qt.AlignVCenter

                            // Subtle outer glow / ring
                            Rectangle {
                                anchors.fill: parent
                                radius: parent.radius
                                color: "transparent"
                                border.color: Qt.rgba(1, 1, 1, 0.35)
                                border.width: 1
                            }
                        }

                        // Tab Title
                        Text {
                            text: modelData.title !== undefined ? modelData.title : "New Tab"
                            color: {
                                if (isActive) return "#FFFFFF";
                                if (isHovered) return "#E2E8F0";
                                return "#94A3B8";
                            }
                            font.pixelSize: 11
                            font.family: "Segoe UI, Inter, -apple-system, sans-serif"
                            font.weight: isActive ? Font.DemiBold : Font.Normal
                            elide: Text.ElideRight
                            Layout.maximumWidth: 140
                            Layout.alignment: Qt.AlignVCenter
                        }

                        // Close Button (✕)
                        Item {
                            id: closeBtnItem
                            z: 10
                            width: 16
                            height: 16
                            Layout.alignment: Qt.AlignVCenter
                            visible: isActive || isHovered

                            Rectangle {
                                id: closeBtnBg
                                anchors.fill: parent
                                radius: 8
                                color: closeArea.containsMouse ? Qt.rgba(0.94, 0.27, 0.27, 0.25) : "transparent"
                                Behavior on color { ColorAnimation { duration: 100 } }

                                Text {
                                    anchors.centerIn: parent
                                    text: "✕"
                                    color: closeArea.containsMouse ? "#EF4444" : (isActive ? "#CBD5E1" : "#64748B")
                                    font.pixelSize: 10
                                }
                            }

                            MouseArea {
                                id: closeArea
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onPressed: mouse.accepted = true
                                onClicked: {
                                    mouse.accepted = true;
                                    if (typeof tabManager !== "undefined") {
                                        tabManager.closeTab(modelData.contextId);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // --- FOOTER: (+) BUTTON ATTACHED DIRECTLY AFTER LAST TAB ---
            footer: Item {
                id: footerItem
                height: tabListView.height
                width: 28

                Rectangle {
                    anchors.centerIn: parent
                    width: 22
                    height: 22
                    radius: 4
                    color: addBtnMouse.containsMouse ? "#2A2D3D" : "transparent"
                    property color borderColor: addBtnMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.16) : "transparent"
                    border.color: borderColor
                    border.width: 1

                    Behavior on color { ColorAnimation { duration: 100 } }
                    Behavior on borderColor { ColorAnimation { duration: 100 } }

                    Text {
                        anchors.centerIn: parent
                        anchors.verticalCenterOffset: -1
                        text: "+"
                        color: addBtnMouse.containsMouse ? "#FFFFFF" : "#94A3B8"
                        font.pixelSize: 14
                        font.weight: Font.Normal
                    }
                }

                MouseArea {
                    id: addBtnMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (typeof tabManager !== "undefined") {
                            tabManager.openNewTab();
                        }
                    }
                }
            }
        }
    }
}