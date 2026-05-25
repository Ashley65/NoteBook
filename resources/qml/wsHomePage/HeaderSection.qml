import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// A reusable component for the top header of the workspace
Item {
    id: root

    // Define the preferred height for the header area
    implicitHeight: mainLayout.implicitHeight
    Layout.fillWidth: true

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        spacing: 18 // Space between the Title row and the Buttons row


        RowLayout {
            Layout.fillWidth: true
            spacing: 5

            // Workspace Title
            Text {
                // Binds to the C++ controller. Updates automatically if changed in C++.
                text: typeof wsHomePage !== "undefined" ? wsHomePage.workspaceName : "Personal Workspace"
                color: "#FFFFFF"
                font.pixelSize: 32
                font.bold: true
                font.family: "Inter"
                Layout.alignment: Qt.AlignVCenter
            }

            Text{
                text: typeof wsHomePage !== "undefined" ? wsHomePage.activeProjectName : "Project Alpha"
                color: "#94A3B8"
                font.pixelSize: 16
                font.family: "Inter"
                Layout.alignment: Qt.AlignVCenter
            }

            // Flexible spacer to push sync status to the far right
            // Item {
            //     Layout.fillWidth: true
            // }

        //     // --- Sync Status Indicators ---
        //     RowLayout {
        //         spacing: 16
        //         Layout.alignment: Qt.AlignVCenter
        //
        //         // Local SQLite Status
        //         RowLayout {
        //             spacing: 6
        //             Rectangle {
        //                 width: 10; height: 10; radius: 5
        //                 color: "#22C55E" // Active Green
        //             }
        //             Text { text: "Local"; color: "#94A3B8"; font.pixelSize: 12 }
        //         }
        //
        //         // LAN Sync Status
        //         RowLayout {
        //             spacing: 6
        //             Rectangle {
        //                 width: 10; height: 10; radius: 5
        //                 color: "#3B82F6" // Active Blue
        //
        //                 // Subtle pulsing animation for LAN
        //                 SequentialAnimation on opacity {
        //                     loops: Animation.Infinite
        //                     NumberAnimation { to: 0.4; duration: 1000 }
        //                     NumberAnimation { to: 1.0; duration: 1000 }
        //                 }
        //             }
        //             Text { text: "LAN (1 Peer)"; color: "#94A3B8"; font.pixelSize: 12 }
        //         }
        //
        //         // Azure Cloud Status
        //         RowLayout {
        //             spacing: 6
        //             Rectangle {
        //                 width: 10; height: 10; radius: 5
        //                 color: "#06B6D4" // Active Cyan
        //             }
        //             Text { text: "Cloud: Synced"; color: "#94A3B8"; font.pixelSize: 12 }
        //         }
        //     }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            // --- NEW TASK BUTTON ---
            Button {
                id: btnNewTask
                text: "+  New Task"

                onClicked: {
                    if (typeof wsHomePage !== "undefined") {
                        wsHomePage.createNewTask("Untitled Task")
                    } else {
                        console.log("Mock: New Task Clicked")
                    }
                }

                contentItem: Text {
                    text: btnNewTask.text
                    color: "#E2E8F0"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: 130
                    implicitHeight: 40
                    color: btnNewTask.hovered ? Qt.rgba(1, 1, 1, 0.15) : Qt.rgba(1, 1, 1, 0.05)
                    radius: 8
                    border.color: "#3B4048"
                    border.width: 1

                    // Smooth color transition on hover
                    Behavior on color { ColorAnimation { duration: 150 } }
                }
            }

            // --- NEW NOTE BUTTON ---
            Button {
                id: btnNewNote
                text: "📄  New Note"

                onClicked: {
                    if (typeof wsHomePage !== "undefined") {
                        wsHomePage.createNewNote("Untitled Note")
                    } else {
                        console.log("Mock: New Note Clicked")
                    }
                }

                contentItem: Text {
                    text: btnNewNote.text
                    color: "#E2E8F0"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: 130
                    implicitHeight: 40
                    color: btnNewNote.hovered ? Qt.rgba(1, 1, 1, 0.15) : Qt.rgba(1, 1, 1, 0.05)
                    radius: 8
                    border.color: "#3B4048"
                    border.width: 1

                    Behavior on color { ColorAnimation { duration: 150 } }
                }
            }

            // --- UPLOAD FILE BUTTON ---
            Button {
                id: btnUploadFile
                text: "  Upload File"

                onClicked: {
                    if (typeof wsHomePage !== "undefined") {
                        wsHomePage.openUploadDialog()
                    } else {
                        console.log("Mock: Upload File Dialog Triggered")
                    }
                }
                contentItem: Text {
                    text: btnUploadFile.text
                    color: "#E2E8F0"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    implicitWidth: 130
                    implicitHeight: 40
                    color: btnUploadFile.hovered ? Qt.rgba(1, 1, 1, 0.15) : Qt.rgba(1, 1, 1, 0.05)
                    radius: 8
                    border.color: "#3B4048"
                    border.width: 1

                    Behavior on color { ColorAnimation { duration: 150 } }
                }
            }
        }
    }
}