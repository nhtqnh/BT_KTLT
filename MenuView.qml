import QtQuick
import QtQuick.Controls

Item {
    id: root

    width: 450
    height: 700

    signal startGame()

    Image {
        anchors.fill: parent
        source: "assets/backgr.png"
        fillMode: Image.PreserveAspectCrop
    }

    Image {
        id: startBtn
        source: "assets/button.png"

        // Giữ nguyên tỷ lệ gốc của ảnh nút bấm
        width: 350
        height: 400
        fillMode: Image.PreserveAspectFit

        // Định vị đưa nút vào giữa màn hình theo chiều dọc
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -45

        // Hiệu ứng nhấn
        scale: buttonMouseArea.pressed ? 0.95 : 1.0
        Behavior on scale { NumberAnimation { duration: 50 } }

        MouseArea {
            id: buttonMouseArea
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                root.startGame()
            }
        }
    }
}