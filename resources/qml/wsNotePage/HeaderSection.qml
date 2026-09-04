import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


Item {
    id: root

    implicitHeight: mainLayout.implicitHeight
    Layout.fillWidth: true

    RowLayout {
        id: mainHeaderLayout
        anchors.fill: parent
        spacing: 20

        RowLayout{
            spacing: 10

            Text {
                text: "Notes"
                font.pixelSize: 22
                font.bold: true
                color: root.textMain
                font.family: "Segoe UI, Inter, sans-serif"
            }

            Rectangle{
                visible: typeof noteListPage !== "undefined" && noteListPage.isProjectScoped
                height: 24
                width: scopeRow.implicitWidth + 20
                radius: 12
                color: Qt.rgba(0.06, 0.72, 0.51, 0.15)
                border.color: Qt.rgba(0.06, 0.72, 0.51, 0.35)
                border.width: 1

                RowLayout {
                    id: scopeRow
                    anchors.centerIn: parent
                    spacing: 6

                    Rectangle {
                        width: 6
                        height: 6
                        radius: 3
                        color: typeof noteListPage !== "undefined" ? noteListPage.projectColor : root.accentGreen
                    }

                    Text {
                        text: typeof noteListPage !== "undefined" ? noteListPage.projectName : ""
                        color: root.accentGreen
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }
                }

                Item { Layout.fillWidth: true } // Spacer

                // Search Bar
                Rectangle {
                    width: 240
                    height: 34
                    radius: 6
                    color: root.surfaceDark
                    border.color: searchInput.activeFocus ? root.accentBlue : root.borderDark
                    border.width: 1

                    RowLayout{
                        anchors.fill: parent
                        anchors.margins: 6
                        spacing: 6

                        Image {
                            source: "qrc:/icons/search.svg"
                            width: 16
                            height: 16
                            fillMode: Image.PreserveAspectFit
                            color: root.textMuted
                        }

                        TextInput {
                            id: searchInput
                            placeholderText: "Search notes..."
                            font.pixelSize: 12
                            color: root.textMain
                            background: null
                            anchors.verticalCenter: parent.verticalCenter
                            Layout.fillWidth: true

                            onTextChanged: {
                                if (typeof noteListPage !== "undefined") {
                                    noteListPage.filterNotes(text)
                    }
                }

            }
        }


    }

}