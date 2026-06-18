import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as Controls


Rectangle{

    id: topBarContainer
    property bool darkmode: true

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

    // radius: 10
    // border.color: "#3B4048"
    // border.width: 1

    ListModel {
        id: tabModel
        ListElement { title: "Workspace Group"; projectColor: "#EF4444" }
        ListElement { title: "Project: Test One"; projectColor: "#3B82F6" }
        ListElement { title: "Project: General"; projectColor: "#EF4444" }
        ListElement { title: "ChronoTasks Dashboard"; projectColor: "#F59E0B" }
    }

    function generateUUID() {
        return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
            var r = Math.random() * 16 | 0, v = c === 'x' ? r : (r & 0x3 | 0x8);
            return v.toString(16);
        });
    }

    Controls.TabBar {
        id: customTabBar
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.topMargin: 4
        anchors.bottomMargin: 4
        spacing: 8

        background: Rectangle { color: "transparent" }

        Repeater{
            model: tabManager.tabs
            delegate: Controls.TabButton {
                id: tabBtn

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: contentRow.implicitWidth + 32

                onClicked: tabManager.activeTabId = modelData.contextId
                checked: modelData.contextId === tabManager.activeTabId


                hoverEnabled: true

                contentItem: Item {
                    RowLayout {
                        id: contentRow
                        anchors.centerIn: parent
                        spacing: 8

                        // The Project Color Dot
                        Rectangle {
                            width: 8
                            height: 8
                            radius: 4
                            color: modelData.projectColour
                            Layout.alignment: Qt.AlignVCenter
                        }

                        // The Tab Text
                        Text {
                            text:  modelData.title
                            color: tabBtn.checked ? "#FFFFFF" : "#A1A1AA"
                            font.pixelSize: 14
                            font.weight: tabBtn.checked ? Font.DemiBold : Font.Normal
                            Layout.alignment: Qt.AlignVCenter
                        }

                        // Close button
                        Item {
                            width: 18
                            height: parent.height
                            Layout.alignment: Qt.AlignVCenter

                            Text {
                                id: closeLabel
                                anchors.centerIn: parent
                                text: "✕"
                                color: tabBtn.hovered ? "#FFFFFF" : "#A1A1AA"
                                font.pixelSize: 12
                                visible: tabBtn.hovered || tabBtn.checked
                            }

                            MouseArea {
                                anchors.fill: parent
                                z: 1
                                hoverEnabled: true
                                propagateComposedEvents: false
                                onClicked: (mouse) => {
                                    // Request the TabManager to close this tab
                                    tabManager.closeTab(modelData.contextId)
                                    console.log("Mock: Close Tab Clicked")
                                    mouse.accepted = true
                                }
                                onPressed: (mouse) => {
                                    // consume event so TabButton.onClicked doesn't fire
                                    mouse.accepted = true
                                }
                            }
                        }
                    }
                }
                background: Rectangle {
                    radius: 20 // This creates the fully rounded pill shape

                    // Active tabs get a lighter background, inactive get a subtle outline
                    color: tabBtn.checked ? "#27272A" : "transparent"
                    border.color: tabBtn.checked ? "#3F3F46" : "#27272A"
                    border.width: 1

                    // Subtle hover effect for inactive tabs
                    Rectangle {
                        anchors.fill: parent
                        radius: 20
                        color: "#FFFFFF"
                        opacity: tabBtn.hovered && !tabBtn.checked ? 0.05 : 0.0
                        Behavior on opacity {
                            NumberAnimation {
                                duration: 150
                            }
                        }
                    }
                }
            }
        }

        Controls.TabButton{
            id: addNewTabBtn
            width: 40
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            text: "+"


            onClicked: {
                tabManager.addTab("New Tab", "Home", generateUUID(), "#3B82F6")
            }

            background: Rectangle {
                radius: 20
                color: addNewTabBtn.hovered ? "#3B82F6" : "transparent"

            }
        }
    }

}