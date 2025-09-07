import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    id: win
    visible: true
    width: 900
    height: 560
    title: "Split + Menu (modular)"

    // 顶部菜单（独立文件）
    menuBar: JMenu {
        id: menubar
        histogramEnabled: right.ready
        onOpenImage: fileDlg.open()
        onHistogram: histDlg.open()
    }

    // 打开图片
    FileDialog {
        id: fileDlg
        title: "Open Image"
        nameFilters: [
            "Images (*.png *.jpg *.jpeg *.bmp *.gif)",
            "All files (*)"
        ]
        onAccepted: if (selectedFile) right.imageUrl = selectedFile
    }

    // 主体：可拖动左右分栏
    SplitView {
        id: split
        anchors.fill: parent

        JLeftPanel { }

        JRightPanel { id: right }

        handle: Rectangle {
            implicitWidth: 6
            color: ma.containsMouse ? "#a0b3ff" : "#d9dde5"
            MouseArea {
                id: ma
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.SplitHCursor
            }
        }
    }

    // 直方图对话框（最小实现）
    Dialog {
        id: histDlg
        title: "Histogram"
        modal: true
        standardButtons: Dialog.Ok

        property var hist: new Array(256).fill(0)

        // 离屏 Canvas 取像素
        Canvas {
            id: offscreen
            visible: false
            width: 256
            height: 256
            onPaint: {
                if (!right.ready) return
                var ctx = getContext("2d"); ctx.reset()
                // 直接用 RightPanel 暴露的 imageItem
                ctx.drawImage(right.imageItem, 0, 0, width, height)
                var d = ctx.getImageData(0, 0, width, height).data
                var arr = new Array(256).fill(0)
                for (var i = 0; i < d.length; i += 4) {
                    var y = Math.round(0.299*d[i] + 0.587*d[i+1] + 0.114*d[i+2])
                    arr[y]++
                }
                histDlg.hist = arr
                display.requestPaint()
            }
        }

        // 可见直方图
        contentItem: Canvas {
            id: display
            width: 640
            height: 280
            onPaint: {
                var ctx = getContext("2d"); ctx.reset()
                ctx.fillStyle = "#f8f8f8"; ctx.fillRect(0,0,width,height)
                ctx.strokeStyle = "#333"; ctx.strokeRect(0.5,0.5,width-1,height-1)
                var h = histDlg.hist, maxv = 0
                for (var i=0; i<256; ++i) if (h[i]>maxv) maxv=h[i]
                if (maxv<=0) return
                var barW = width/256; ctx.fillStyle = "#4a90e2"
                for (var x=0; x<256; ++x) {
                    var bh = Math.max(1, Math.round((height-2) * (h[x]/maxv)))
                    ctx.fillRect(x*barW, height-1-bh, barW, bh)
                }
            }
        }

        onOpened: if (right.ready) offscreen.requestPaint()
    }
}
