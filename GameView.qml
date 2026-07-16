import QtQuick
import QtQuick.Controls
import QtQuick 2.15
import QtMultimedia

Item {
    id: gameRoot

    width: 400
    height: 380
    focus: true

    property int scrollSpeed: 5
    property var lanePositions: [131, 229]
    property int currentLaneIndex: 0
    property int currentCoins: 0
    property bool isShieldActive: false
    property bool isPaused: false

    signal startGame()
    signal returnToMenu()
    signal restartRequested()
    signal shieldActivated()
    signal shieldDeactivated()

    // --- BACKGROUND TỰ CUỘN ---
    Image {
        id: bg1
        width: parent.width; height: parent.height
        y: 0
        source: "assets/main.png"
        fillMode: Image.Stretch
    }

    Image {
        id: bg2
        width: parent.width; height: parent.height
        y: -parent.height
        source: "assets/main.png"
        fillMode: Image.Stretch
    }

    Timer {
        id: gameTimer
        interval: 16
        repeat: true
        running: gameRoot.visible && !gameRoot.isPaused

        onTriggered: {
            bg1.y += gameRoot.scrollSpeed
            bg2.y += gameRoot.scrollSpeed

            if (bg1.y >= gameRoot.height) bg1.y = bg2.y - gameRoot.height
            if (bg2.y >= gameRoot.height) bg2.y = bg1.y - gameRoot.height
        }
    }
    // --- XE NGƯỜI CHƠI ---
    Item {
        id: playerCar
        width: 90; height: 110
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 30
        x: gameRoot.lanePositions[gameRoot.currentLaneIndex]

        // 🌟 SỬA TẠI ĐÂY: Nếu bật khiên, gửi tọa độ giả ra ngoài màn hình để C++ không tính va chạm
        onXChanged: {
            if (!gameRoot.isPaused) {
                if (gameRoot.isShieldActive)
                    gameEngine.updatePlayerPosition(-1000, -1000)
                else
                    gameEngine.updatePlayerPosition(x, y)
            }
        }
        onYChanged: {
            if (!gameRoot.isPaused) {
                if (gameRoot.isShieldActive)
                    gameEngine.updatePlayerPosition(-1000, -1000)
                else
                    gameEngine.updatePlayerPosition(x, y)
            }
        }

        Behavior on x {
            NumberAnimation { duration: 120; easing.type: Easing.OutQuad }
        }

        Image {
            anchors.fill: parent
            source: "assets/car.png"
            fillMode: Image.PreserveAspectFit
            visible: !gameRoot.isShieldActive
        }

        Image {
            anchors.fill: parent
            source: "assets/car_1.png"
            fillMode: Image.PreserveAspectFit
            visible: gameRoot.isShieldActive
        }
    }

    // --- VẬT CẢN ---
    Repeater {
        model: gameEngine.obstacleModel

        Image {
            x: obstacle.x
            y: obstacle.y
            width: obstacle.getWidth()
            height: obstacle.getHeight()
            source: obstacle.imagePath()
            fillMode: Image.PreserveAspectFit
            z: 5

            Behavior on y {
                enabled: !gameRoot.isPaused
                NumberAnimation {
                    duration: 16
                    easing.type: Easing.Linear
                }
            }
        }
    }

    // --- THANH HIỂN THỊ XU ---
    Row {
        id: coinUI
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 15
        spacing: 10
        z: 90

        Image {
            source: "assets/coin.png"
            width: 35; height: 35
            fillMode: Image.PreserveAspectFit
        }

        Text {
            text: gameRoot.currentCoins
            font.pixelSize: 26
            font.bold: true
            color: gameRoot.currentCoins >= 3 ? "#00FF00" : "#FFD700"
            style: Text.Outline
            styleColor: "white"
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    // --- NÚT PAUSE GAME (GÓC TRÊN PHẢI) ---
    Image {
        id: pauseBtn
        source: "assets/pause.png"
        width: 70; height: 70
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 15
        z: 90
        fillMode: Image.PreserveAspectFit

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (!gameOverOverlay.visible) {
                    gameRoot.isPaused = true
                    gameEngine.pauseGame()
                    bgMusic.stop()
                }
            }
        }
    }

    // --- XỬ LÝ VA CHẠM TỪ C++ ---
    Connections {
        target: gameEngine

        function onGameOverChanged() {
            if (gameEngine.isGameOver && !gameRoot.isPaused) {
                gameOverOverlay.visible = true
            }
        }

        function onCrashed() {
            if (!gameRoot.isPaused) {
                gameOverOverlay.visible = true
            }
        }

        function onCoinCollected() {
            if (!gameRoot.isPaused) gameRoot.currentCoins++;
        }
    }

    // --- TIMER  ---
        Timer {
            id: shieldTimer
            interval: 5000
            repeat: false
            running: !gameRoot.isPaused && gameRoot.isShieldActive
            onTriggered: {
                gameRoot.isShieldActive = false
                gameRoot.shieldDeactivated()
                gameEngine.updatePlayerPosition(playerCar.x, playerCar.y)
            }
        } // <-- PHẢI CÓ DẤU NGOẶC NÀY ĐỂ ĐÓNG TIMER KHIÊN LẠI

        // --- TIMER TỰ ĐỘNG TĂNG TỐC ĐỘ DẦN ĐỀU ---
        Timer {
            id: speedUpTimer
            interval: 5000
            repeat: true
            running: gameRoot.visible && !gameRoot.isPaused // Đã sửa lỗi chính tả unning -> running

            onTriggered: {
                if (gameRoot.scrollSpeed < 18) {
                    gameRoot.scrollSpeed += 1
                    console.log("Tốc độ hiện tại: " + gameRoot.scrollSpeed)
                }
            }
        }

    // =========================================================
    // 🌟 MÀN HÌNH TẠM DỪNG (PAUSE OVERLAY) - BẢN ĐỒNG BỘ 2 NÚT TRÒN
    // =========================================================
    Item {
        id: pauseOverlay
        anchors.fill: parent
        visible: gameRoot.isPaused
        z: 100

        Rectangle {
            anchors.fill: parent
            color: "#80000000"
        }

        MouseArea { anchors.fill: parent }

        Row {
            anchors.centerIn: parent
            spacing: -40 // Vì ảnh gốc có viền tàng hình rộng, đặt spacing âm sẽ kéo 2 nút thật lại sát nhau rất đẹp

            // 1. NÚT TIẾP TỤC (RESUME)
            Image {
                id: resumeBtn
                source: "assets/resume.png"
                width: 220; height: 220 // Kích thước đồng bộ lớn để hiện rõ nút bên trong
                fillMode: Image.PreserveAspectFit

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        gameRoot.isPaused = false
                        gameEngine.resumeGame()
                        bgMusic.play()
                    }
                }
            }

            // 2. NÚT THOÁT MỚI (HÌNH TRÒN ĐỒNG BỘ)
            Image {
                id: pauseExitBtn
                source: "assets/resume_1.png" // 🌟 Fen nhớ bỏ ảnh tròn mới vào assets và đổi tên giống thế này nhé
                width: 220; height: 220 // Set kích thước BẰNG KHÍT nút Resume để đối xứng 100%
                fillMode: Image.PreserveAspectFit

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        gameRoot.isPaused = false
                        gameRoot.returnToMenu()
                    }
                }
            }
        }
    }

    // =========================================================
    // --- MÀN HÌNH GAME OVER ---
    // =========================================================
    Item {
        id: gameOverOverlay
        anchors.fill: parent
        visible: false
        z: 99

        MouseArea { anchors.fill: parent }

        Image {
            id: gameOverTitle
            source: "assets/over.png"
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
        }

        Image {
            id: restartBtn
            source: "assets/replay.png"
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
        }

        Image {
            id: exitBtn
            source: "assets/exit.png"
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
        }

        MouseArea {
            id: realRestartClick
            width: 160; height: 60
            x: (parent.width - width) / 2
            y: 320
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                gameEngine.restartGame()
                gameOverOverlay.visible = false
                gameRoot.restartRequested()
                gameRoot.currentCoins = 0
                gameRoot.scrollSpeed = 5
                gameRoot.isShieldActive = false
                gameRoot.isPaused = false
                shieldTimer.stop()
            }
        }

        MouseArea {
            id: realExitClick
            width: 160; height: 60
            x: (parent.width - width) / 2
            y: 430
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                gameOverOverlay.visible = false
                gameRoot.returnToMenu()
            }
        }
    }

    // --- ĐIỀU KHIỂN BÀN PHÍM ---
        Keys.onPressed: (event) => {
            if (gameRoot.isPaused) return;

            if (event.key === Qt.Key_Left) {
                if (gameRoot.currentLaneIndex > 0) gameRoot.currentLaneIndex--;
            }
            else if (event.key === Qt.Key_Right) {
                if (gameRoot.currentLaneIndex < 1 ) gameRoot.currentLaneIndex++;
            }
            // 🌟 ĐÃ XÓA MŨI TÊN LÊN/XUỐNG Ở ĐÂY
            else if (event.key === Qt.Key_Space) {
                if (gameRoot.currentCoins >= 3 && !gameRoot.isShieldActive) {
                    gameRoot.currentCoins -= 3;
                    gameRoot.isShieldActive = true;

                    // PHÁT TÍN HIỆU BẬT KHIÊN RA NGOÀI
                    gameRoot.shieldActivated();

                    gameEngine.updatePlayerPosition(-1000, -1000);
                    shieldTimer.start();
                }
            }
        }
}