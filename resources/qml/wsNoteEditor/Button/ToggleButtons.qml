import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Row {
    id: control
    spacing: 4

    property string currentMode: "split"
    property color iconColor: "#D4D4D4"

    // Pass the selected mode string up to the header
    signal viewModeChanged(string mode)

    ButtonGroup {
        id: viewModeGroup
        onClicked: function(button) {
            control.currentMode = button.modeValue
            control.viewModeChanged(button.modeValue) // Emit the new mode!
        }
    }

    ToolButton {
        id: editBtn
        property string modeValue: "edit"

        ButtonGroup.group: viewModeGroup
        checkable: true
        checked: control.currentMode === modeValue

        icon.source: "qrc:/icons/edit-icon.svg"
        icon.color: control.iconColor
        implicitWidth: 36
        implicitHeight: 36

        background: Rectangle {
            radius: 6
            // Fixed typo: was editorBtn.checked, is now editBtn.checked
            color: editBtn.checked ? "#333333" : (editBtn.hovered ? "#2A2A2A" : "transparent")
        }
    }

    ToolButton {
        id: splitBtn
        property string modeValue: "split"

        ButtonGroup.group: viewModeGroup
        checkable: true
        checked: control.currentMode === modeValue

        icon.source: "qrc:/icons/split_view.svg"
        icon.color: control.iconColor
        implicitWidth: 36
        implicitHeight: 36

        background: Rectangle {
            radius: 6
            color: splitBtn.checked ? "#333333" : (splitBtn.hovered ? "#2A2A2A" : "transparent")
        }
    }

    ToolButton {
        id: previewBtn
        property string modeValue: "preview"

        ButtonGroup.group: viewModeGroup
        checkable: true
        checked: control.currentMode === modeValue

        icon.source: "qrc:/icons/image.svg"
        icon.color: control.iconColor
        implicitWidth: 36
        implicitHeight: 36

        background: Rectangle {
            radius: 6
            color: previewBtn.checked ? "#333333" : (previewBtn.hovered ? "#2A2A2A" : "transparent")
        }
    }
}