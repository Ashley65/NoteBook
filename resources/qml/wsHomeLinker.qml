import QtQuick 2.15
import QtQuick.Layouts 1.15
import "wsHomePage" as WsHome

Item {
    id: root
    anchors.fill: parent


    ColumnLayout {
        anchors.fill: parent
        spacing: 24

        WsHome.HeaderSection {
            Layout.fillWidth: true
        }

        RowLayout{
            Layout.fillWidth: true
            spacing: 16

            WsHome.RecentNotesColumn {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: 400
            }

            WsHome.TaskFocusColumn {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: 400
            }
        }

    }
}
