import QtQuick 2.15
import QtQuick.Layouts 1.15
import "wsTaskBoardPage" as WsTaskBoard

Item {
    id: root
    anchors.fill: parent

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // Header Section: Title, Search, Filters, Add Button
        WsTaskBoard.BoardHeader {
            Layout.fillWidth: true
        }

        // Kanban Board: 3 Columns
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            // To Do Column
            WsTaskBoard.KanbanColumn {
                title: "To Do"
                statusColor: "#3B82F6"
                targetStatus: 0
                count: typeof taskBoard !== "undefined" ? taskBoard.todoCount : 0
                taskModel: typeof taskBoard !== "undefined" ? taskBoard.todoTasks : []
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            // In Progress Column
            WsTaskBoard.KanbanColumn {
                title: "In Progress"
                statusColor: "#F59E0B"
                targetStatus: 1
                count: typeof taskBoard !== "undefined" ? taskBoard.inProgressCount : 0
                taskModel: typeof taskBoard !== "undefined" ? taskBoard.inProgressTasks : []
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            // Completed Column
            WsTaskBoard.KanbanColumn {
                title: "Completed"
                statusColor: "#10B981"
                targetStatus: 2
                count: typeof taskBoard !== "undefined" ? taskBoard.completedCount : 0
                taskModel: typeof taskBoard !== "undefined" ? taskBoard.completedTasks : []
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
