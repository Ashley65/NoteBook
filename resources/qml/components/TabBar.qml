import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls

Rectangle {
    id: topBarContainer
    property bool darkmode: true

    // Background Styling
    gradient: Gradient {
        GradientStop {
            position: 0.0;
            color: topBarContainer.darkmode ? "#1e1e1e" : "#f7f7f7"
        }
        GradientStop {
            position: 1.0;
            color: topBarContainer.darkmode ? "#161616" : "#efefef"
        }
    }
    color: topBarContainer.darkmode ? Qt.rgba(1, 1, 1, 0.04) : Qt.rgba(0, 0, 0, 0.06)

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.topMargin: 4
        anchors.bottomMargin: 4
        spacing: 8

        // ---------------------------------------------------------
        // 1. The Dynamic Tab List (Replacing Controls.TabBar)
        // ---------------------------------------------------------
        ListView {
            id: tabListView
            Layout.fillWidth: true
            Layout.fillHeight: true

            orientation: ListView.Horizontal
            spacing: 8
            clip: true // Hides tabs that overflow the screen width

            // Connect safely to your C++ Brain
            model: typeof tabManager !== "undefined" ? tabManager.tabs : null

            // Smoothly animate tabs appearing and disappearing
            add: Transition { NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 200 } }
            remove: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 200 } }
            displaced: Transition { NumberAnimation { properties: "x,y"; duration: 200 } }

            delegate: Item {
                id: tabDelegate
                height: ListView.view.height
                width: contentRow.implicitWidth + 32

                // State helpers
                property bool isActive: typeof tabManager !== "undefined" && tabManager.activeTabId === modelData.contextId
                property bool isHovered: hoverArea.containsMouse

                // The Pill Background
                Rectangle {
                    anchors.fill: parent
                    radius: height / 2
                    color: isActive ? "#27272A" : "transparent"
                    border.color: isActive ? "#3F3F46" : "#27272A"
                    border.width: 1

                    // Subtle hover effect for inactive tabs
                    Rectangle {
                        anchors.fill: parent
                        radius: parent.radius
                        color: "#FFFFFF"
                        opacity: isHovered && !isActive ? 0.05 : 0.0
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }
                }

                // Main Tab Click Area
                MouseArea {
                    id: hoverArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        if (typeof tabManager !== "undefined") {
                            tabManager.activeTabId = modelData.contextId;
                        }
                    }
                }

                // Tab Content (Dot, Text, Close Button)
                RowLayout {
                    id: contentRow
                    anchors.centerIn: parent
                    spacing: 8

                    // Project Color Dot
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: modelData.projectColour !== undefined ? modelData.projectColour : "#A1A1AA"
                        Layout.alignment: Qt.AlignVCenter
                    }

                    // Tab Title
                    Text {
                        text: modelData.title !== undefined ? modelData.title : "Unknown"
                        color: isActive ? "#FFFFFF" : "#A1A1AA"
                        font.pixelSize: 14
                        font.weight: isActive ? Font.DemiBold : Font.Normal
                        Layout.alignment: Qt.AlignVCenter
                    }

                    // Close Button
                    Item {
                        width: 18
                        height: 18
                        Layout.alignment: Qt.AlignVCenter
                        visible: isHovered || isActive

                        Text {
                            anchors.centerIn: parent
                            text: "✕"
                            // Turn red when hovering specifically over the close button
                            color: closeArea.containsMouse ? "#EF4444" : (isActive ? "#FFFFFF" : "#A1A1AA")
                            font.pixelSize: 12
                        }

                        MouseArea {
                            id: closeArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            // Prevent click from leaking to the main tab area
                            onPressed: mouse.accepted = true

                            onClicked: {
                                if (typeof tabManager !== "undefined") {
                                    tabManager.closeTab(modelData.contextId);
                                }
                            }
                        }
                    }
                }
            }
        }

        // ---------------------------------------------------------
        // 2. The Add New Tab Button
        // ---------------------------------------------------------
        Item {
            Layout.preferredWidth: 40
            Layout.fillHeight: true

            Rectangle {
                anchors.fill: parent
                radius: height / 2
                color: addNewTabBtnArea.containsMouse ? "#3B82F6" : "transparent"
                Behavior on color { ColorAnimation { duration: 150 } }

                Text {
                    anchors.centerIn: parent
                    text: "+"
                    color: addNewTabBtnArea.containsMouse ? "#FFFFFF" : "#A1A1AA"
                    font.pixelSize: 18
                }
            }

            MouseArea {
                id: addNewTabBtnArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (typeof tabManager !== "undefined") {
                        // FIX: Use a constant zeroed UUID instead of generating a random one
                        // This allows C++ findTabIndexByContextId to prevent duplicates!
                        tabManager.addTab("New Tab", "Dashboard", "{00000000-0000-0000-0000-000000000000}", "#3B82F6");
                    }
                }
            }
        }
    }
}