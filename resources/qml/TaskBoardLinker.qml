import QtQuick 2.15
import QtQuick.Layouts 1.15
import "taskBoardPage" as TaskBoard



Item {
    id: root
    anchors.fill: parent


    ColumnLayout {
        anchors.fill: parent
        spacing: 24

        TaskBoard.HeaderSection {
            Layout.fillWidth: true
        }

        RowLayout{
            Layouts.fillWidth: true
            spacing: 16


        }

    }
}