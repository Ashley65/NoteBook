import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: sidebarRoot
    // Visual Properties
    color: "#101015" // Matches your dark theme background
    width: isCompact ? 60 : 260

    // Logic Properties
    readonly property bool isCompact: sideBar.isCompact
    readonly property string activeProjectIdString: sideBar.activeProjectId.toString().replace(/[{}]/g, "").toLowerCase()

    // Debug output
    Component.onCompleted: {
        console.log("SideBar initialized - isCompact:", isCompact, "width:", width)
    }

    onIsCompactChanged: {
        console.log("isCompact changed to:", isCompact, "width:", width)
    }

    // Animation for width changes
    Behavior on width { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // --- 0. WORKSPACE CONTEXT SECTION ---
        SidebarItem {
            Layout.topMargin: 10
            text: sideBar.workspaceName
            iconSource: "🏠"
            compact: sidebarRoot.isCompact
            onClicked: sideBar.onPrimaryClicked()
        }

        // --- 1. CORE NAVIGATION SECTION ---
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: 20
            spacing: 2

            Repeater {
                model: ListModel {
                    ListElement {name: "Inbox"; icon: "📥"; active: false}
                    ListElement { name: "Today"; icon: "⭐"; active: true } // Selected in your image
                    ListElement { name: "Upcoming"; icon: "📅"; active: false }
                    ListElement { name: "Dashboard"; icon: "📊"; active: false }
                }

                delegate: SidebarItem {
                    text: model.name || ""
                    iconSource: model.icon || ""
                    isSelected: model.active || false
                    compact: sidebarRoot.isCompact
                    onClicked: sideBar.onItemClicked("core", model.name || "")
                }
            }
        }

        // --- 2. SCROLLABLE CONTENT (Projects & Filters) ---
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 20
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            Column { // CHANGED from ColumnLayout to Column
                width: sidebarRoot.width - 20
                spacing: 20

                // PROJECTS GROUP
                SidebarSection {
                    width: parent.width
                    title: "Projects"
                    compact: sidebarRoot.isCompact

                    model: sideBar.projects
                    //isActive: modelData.id === sideBar.activeProjectId.toString().replace(/[{}]/g, "")

                    Button {

                        id: btnNewProject
                        text: "+ New Project"
                        hoverEnabled: true

                        visible: !sidebarRoot.isCompact
                        enabled: visible
                        Layout.fillWidth: true
                        Layout.preferredHeight: visible ? implicitHeight : 0
                        Layout.maximumHeight: visible ? implicitHeight : 0

                        background: Rectangle {
                            color: btnNewProject.hovered ? "#1a1f2e" : "transparent"
                            radius: 6

                            Behavior on color { ColorAnimation { duration: 150 } }
                        }
                        contentItem: Text {
                            text: parent.text
                            color: btnNewProject.hovered ? "#9bb8ff" : "#7aa2f7"
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: sideBar.onAddProject()
                    }

                    delegate: SidebarItem {
                        width: sidebarRoot.width - 20
                        text: modelData.name || ""
                        isSelected: (modelData.id || "").toLowerCase() === sidebarRoot.activeProjectIdString
                        useAvatar: true
                        avatarColor: modelData.colorCode || "transparent"
                        avatarText: (modelData.name || "").substring(0, 2).toUpperCase()
                        compact: sidebarRoot.isCompact
                        onClicked: sideBar.onItemClicked("project", modelData.id || "")
                    }
                }

                // FILTERS GROUP
                SidebarSection {
                    width: parent.width
                    title: "Filters"
                    compact: sidebarRoot.isCompact

                    model: ListModel {
                        ListElement { name: "High Priority"; colorCode: "#FF4500"; code: "HP"; icon: "" }
                        ListElement { name: "Overdue"; colorCode: "#FF8C00"; code: "OV"; icon: "" }
                        ListElement { name: "No Due Date"; colorCode: "#A9A9A9"; code: "ND"; icon: "" }
                        ListElement { name: "Completed"; colorCode: "#32CD32"; code: "OK"; icon: "" }
                    }

                    delegate: SidebarItem {
                        width: sidebarRoot.width - 20 // FORCE width for delegate
                        text: model.name || ""
                        useAvatar: true
                        avatarColor: model.colorCode || "transparent"
                        avatarText: sidebarRoot.isCompact ? (model.code || "") : "🏳️" // Flag or code
                        compact: sidebarRoot.isCompact
                        onClicked: sideBar.onItemClicked("filter", model.name || "")
                    }
                }
            }
        }

        // --- 3. FOOTER SECTION ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15

                // Hamburger / Sidebar Toggle
                Button {
                    text: "☰"
                    background: Rectangle { color: "transparent" }
                    contentItem: Text {
                        text: parent.text; color: "#7aa2f7"; font.pixelSize: 20
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    Layout.preferredWidth: 40
                    onClicked: sideBar.onToggleMode()
                }

                // Footer Text (Hidden in compact)
                ColumnLayout {
                    visible: !sidebarRoot.isCompact
                    Layout.fillWidth: true
                    opacity: visible ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: 150 } }

                    Text {
                        text: ""
                        color: "#c0caf5"
                        font.bold: true
                        font.pixelSize: 14
                    }
                    RowLayout {
                        Rectangle { width: 6; height: 6; radius: 3; color: "#9ece6a" } // Green dot
                        Text { text: "Synced"; color: "#565f89"; font.pixelSize: 12 }
                    }
                }
            }
        }
    }

    // --- HELPER COMPONENT: Sidebar Item (The Row) ---
    component SidebarItem: Rectangle {
        id: sidebarItem
        property string text: ""
        property string iconSource: ""
        property bool isSelected: false
        property bool compact: false
        property bool useAvatar: false
        property string avatarText: ""
        property color avatarColor: "transparent"
        signal clicked()

        Layout.fillWidth: true
        Layout.preferredHeight: 40

        // ADD THIS LINE: Explicit fallback to fix QML alias layout bug
        width: parent ? parent.width : 0

        color: isSelected ? "#1f2335" : (hoverArea.containsMouse ? "#16161e" : "transparent")
        radius: 6

        // Hover effect
        MouseArea {
            id: hoverArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: parent.clicked()
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: sidebarItem.compact ? 0 : 12
            anchors.rightMargin: sidebarItem.compact ? 0 : 8
            spacing: sidebarItem.compact ? 0 : 12

            // Icon Area
            Item {
                Layout.preferredWidth: sidebarItem.compact ? sidebarItem.width : 30
                Layout.preferredHeight: 30
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                // Option A: Standard Icon
                Text {
                    visible: !sidebarItem.useAvatar
                    text: sidebarItem.iconSource || ""
                    color: sidebarItem.isSelected ? "#7aa2f7" : "#a9b1d6"
                    font.pixelSize: 18
                    anchors.centerIn: parent
                }

                // Option B: Avatar Circle (For Projects/Filters)
                Rectangle {
                    visible: sidebarItem.useAvatar
                    width: 24; height: 24
                    radius: 12
                    color: "transparent"
                    border.color: sidebarItem.avatarColor || "transparent"
                    border.width: 1
                    anchors.centerIn: parent

                    Text {
                        text: sidebarItem.avatarText || ""
                        anchors.centerIn: parent
                        color: sidebarItem.avatarColor || "transparent"
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            // Text Label (Hidden in compact mode)
            Text {
                id: labelText
                visible: !sidebarItem.compact
                text: sidebarItem.text
                color: "#c0caf5"
                font.pixelSize: 14
                Layout.fillWidth: !sidebarItem.compact  // Don't fill width in compact mode
                Layout.preferredWidth: sidebarItem.compact ? 0 : -1  // 0 width in compact mode
                Layout.alignment: Qt.AlignVCenter
                elide: Text.ElideRight
                wrapMode: Text.NoWrap

                Component.onCompleted: {
                    console.log("Text label created - visible:", visible, "compact:", sidebarItem.compact, "text:", text)
                }
            }
        }
    }

    // --- HELPER COMPONENT: Section Header ---
    component SidebarSection: ColumnLayout {
        property string title
        property bool compact
        property alias model: repeater.model
        property alias delegate: repeater.delegate

        Layout.fillWidth: true
        spacing: 2

        // Header Text ("Projects", "Filters") - Hide in compact
        Text {
            visible: !compact
            text: title
            color: "#565f89"
            font.pixelSize: 12
            font.bold: true
            Layout.leftMargin: 12
            Layout.bottomMargin: 5
        }

        Repeater {
            id: repeater
        }
    }
}