import QtQuick 2.15
import QtQuick.Controls 2.15


Popup{
    id: control
    width: 250
    height: Math.min(contentItem.implicitHeight + 10, 200)
    padding: 5

    signal suggestionSelected(string title)

    function updateAndOpen(popupX, popupY, results) {
        suggestionModel.clear();
        for (var i = 0; i < results.length; i++) {
            suggestionModel.append(results[i]);
        }

        if (suggestionModel.count > 0) {
            control.x = popupX;
            control.y = popupY;
            control.open();
        } else {
            control.close();
        }
    }

    background: Rectangle {
        color: "#252526" // Slightly lighter than background to pop out
        border.color: root.borderDark
        radius: 6
        layer.enabled: true // Drop shadow effect
    }

    contentItem: ListView {
        id: suggestionListView
        clip: true
        spacing: 2
        model: ListModel { id: suggestionModel }

        delegate: ItemDelegate {
            width: ListView.view.width
            height: 35

            contentItem: Text {
                text: model.title
                color: root.textMain
                font.pixelSize: 14
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                color: parent.hovered ? "#37373D" : "transparent"
                radius: 4
            }

            onClicked: {
                // Emit the chosen title back to the editor, then hide!
                control.suggestionSelected(model.title)
                control.close()
            }
        }
    }

}