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

            ColumnLayout {
                width: sidebarRoot.width - 20 // Explicit width based on sidebar width
                spacing: 20

                // PROJECTS GROUP
                SidebarSection {
                    Layout.fillWidth: true
                    title: "Projects"
                    compact: sidebarRoot.isCompact

                    // Example Data
                    model: ListModel {
                        ListElement { name: "System Design"; colorCode: "#FFD700"; icon: "" } // Gold
                        ListElement { name: "Fitness Plan"; colorCode: "#9ACD32"; icon: "" }  // Green
                        ListElement { name: "Dissertation"; colorCode: "#20B2AA"; icon: "" }  // Teal
                        ListElement { name: "Task Monitoring App"; colorCode: "#FF69B4"; icon: "" } // Pink
                    }

                    delegate: SidebarItem {
                        text: model.name || ""
                        // Show avatar circles for projects (colored circles with initials)
                        useAvatar: true  // ← Simplified to always true
                        avatarColor: model.colorCode || "transparent"
                        avatarText: (model.name || "").substring(0, 2).toUpperCase()
                        compact: sidebarRoot.isCompact
                        onClicked: sideBar.onItemClicked("project", model.name || "")
                    }
                }

                // FILTERS GROUP
                SidebarSection {
                    Layout.fillWidth: true
                    title: "Filters"
                    compact: sidebarRoot.isCompact

                    model: ListModel {
                        ListElement { name: "High Priority"; colorCode: "#FF4500"; code: "HP"; icon: "" }
                        ListElement { name: "Overdue"; colorCode: "#FF8C00"; code: "OV"; icon: "" }
                        ListElement { name: "No Due Date"; colorCode: "#A9A9A9"; code: "ND"; icon: "" }
                        ListElement { name: "Completed"; colorCode: "#32CD32"; code: "OK"; icon: "" }
                    }

                    delegate: SidebarItem {
                        text: model.name || ""
                        useAvatar: true // Filters always use the small coloured dot/avatar style
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
        color: isSelected ? "#1f2335" : "transparent" // Selection background
        radius: 6

        // Hover effect
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: parent.color = isSelected ? "#1f2335" : "#16161e"
            onExited: parent.color = isSelected ? "#1f2335" : "transparent"
            onClicked: parent.clicked()
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: sidebarItem.compact ? 0 : 12
            anchors.rightMargin: 8
            spacing: 12
            Layout.alignment: Qt.AlignHCenter

            // Icon Area
            Item {
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                Layout.fillWidth: sidebarItem.compact  // Expand to fill width in compact mode
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter  // Center both horizontally and vertically

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
                visible: sidebarRoot.width > 100
                text: sidebarItem.text
                color: "#c0caf5"
                font.pixelSize: 14
                Layout.fillWidth: sidebarRoot.width > 100  // Don't fill width in compact mode
                Layout.preferredWidth: sidebarRoot.width > 100 ? -1 : 0  // 0 width in compact mode
                Layout.alignment: Qt.AlignVCenter
                elide: Text.ElideRight
                wrapMode: Text.NoWrap

                Component.onCompleted: {
                    console.log("Text label created - visible:", visible, "compact:", sidebarItem.compact, "text:", text, "width:", sidebarRoot.width)
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