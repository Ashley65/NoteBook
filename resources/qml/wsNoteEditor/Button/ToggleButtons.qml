import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: control
    implicitWidth: 196
    implicitHeight: 32
    color: "#12131D"
    border.color: "#2B2D3F"
    border.width: 1
    radius: 6

    property string currentMode: "split"

    signal viewModeChanged(string mode)

    ButtonGroup {
        id: viewModeGroup
        onClicked: function(button) {
            control.currentMode = button.modeValue
            control.viewModeChanged(button.modeValue)
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 2
        spacing: 2

        Button {
            id: btnEdit
            property string modeValue: "edit"
            Layout.fillWidth: true
            Layout.fillHeight: true
            checkable: true
            checked: control.currentMode === "edit"
            ButtonGroup.group: viewModeGroup

            contentItem: Text {
                text: "Edit"
                font.pixelSize: 11
                font.weight: btnEdit.checked ? Font.Bold : Font.Medium
                color: btnEdit.checked ? "#FFFFFF" : (btnEdit.hovered ? "#E2E8F0" : "#94A3B8")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                radius: 4
                color: btnEdit.checked ? "#2B2D3F" : (btnEdit.hovered ? "rgba(255, 255, 255, 0.05)" : "transparent")
                border.color: btnEdit.checked ? "#3E4259" : "transparent"
                border.width: 1
            }
        }

        Button {
            id: btnSplit
            property string modeValue: "split"
            Layout.fillWidth: true
            Layout.fillHeight: true
            checkable: true
            checked: control.currentMode === "split"
            ButtonGroup.group: viewModeGroup

            contentItem: Text {
                text: "Split"
                font.pixelSize: 11
                font.weight: btnSplit.checked ? Font.Bold : Font.Medium
                color: btnSplit.checked ? "#FFFFFF" : (btnSplit.hovered ? "#E2E8F0" : "#94A3B8")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                radius: 4
                color: btnSplit.checked ? "#2B2D3F" : (btnSplit.hovered ? "rgba(255, 255, 255, 0.05)" : "transparent")
                border.color: btnSplit.checked ? "#3E4259" : "transparent"
                border.width: 1
            }
        }

        Button {
            id: btnPreview
            property string modeValue: "preview"
            Layout.fillWidth: true
            Layout.fillHeight: true
            checkable: true
            checked: control.currentMode === "preview"
            ButtonGroup.group: viewModeGroup

            contentItem: Text {
                text: "Preview"
                font.pixelSize: 11
                font.weight: btnPreview.checked ? Font.Bold : Font.Medium
                color: btnPreview.checked ? "#FFFFFF" : (btnPreview.hovered ? "#E2E8F0" : "#94A3B8")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                radius: 4
                color: btnPreview.checked ? "#2B2D3F" : (btnPreview.hovered ? "rgba(255, 255, 255, 0.05)" : "transparent")
                border.color: btnPreview.checked ? "#3E4259" : "transparent"
                border.width: 1
            }
        }
    }
}