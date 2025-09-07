import QtQuick
import QtQuick.Controls

MenuBar {
    id: root
    // 由 Main 绑定控制
    property bool histogramEnabled: false
    signal openImage()
    signal histogram()

    Menu {
        title: "File"
        Action {
            text: "Open Image"
            shortcut: StandardKey.Open
            onTriggered: root.openImage()
        }
    }
    Menu {
        title: "Tools"
        Action {
            text: "Histogram"
            enabled: root.histogramEnabled
            onTriggered: root.histogram()
        }
    }
}
