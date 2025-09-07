import QtQuick
import QtQuick.Controls  // 为使用 SplitView 附加属性

Item {
    id: root
    // 供 SplitView 使用的尺寸建议
    SplitView.minimumWidth: 160
    SplitView.preferredWidth: 260

    Rectangle {
        anchors.fill: parent
        color: "#f3f6fb"
        border.color: "#d0d7de"
        Text { anchors.centerIn: parent; text: "Left Pane" }
    }
}
