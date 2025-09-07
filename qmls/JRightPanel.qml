import QtQuick

Item {
    id: root
    // 对外 API
    property url imageUrl: ""
    readonly property bool ready: img.status === Image.Ready
    // 让外部可引用内部 Image 元素（供 Canvas 绘图）
    property alias imageItem: img

    Rectangle {
        anchors.fill: parent
        color: "#ffffff"
        border.color: "#d0d7de"

        Image {
            id: img
            anchors.fill: parent
            source: root.imageUrl
            fillMode: Image.PreserveAspectFit
            cache: true
            smooth: true
        }

        // 没有图片时的提示
        Text {
            anchors.centerIn: parent
            visible: !root.ready
            color: "#888"
            text: "Use File → Open Image"
        }
    }
}
