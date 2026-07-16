import QtQuick
import QtQuick.Controls
import QtQuick 2.15
import QtMultimedia

ApplicationWindow {
    id: window
    visible: true

    width: 450
    height: 700

    minimumWidth: 450
    minimumHeight: 700
    maximumWidth: 450
    maximumHeight: 700

    title: qsTr("Car Game Racing")

    MenuView {
        id: menuView
        anchors.fill: parent
        visible: true // Mới mở app lên thì Menu hiện

        onStartGame: {
            menuView.visible = false // Ẩn Menu
            gameView.visible = true  // Hiện Game
            gameEngine.startGame()   // Báo C++ reset điểm, sinh xe

            // 🌟 ĐỒNG BỘ NHẠC: Reset nhạc về bài đầu tiên khi bắt đầu ván mới hoàn toàn từ Menu
            bgMusic.currentMusicIndex = 0
            bgMusic.playRandomMusic()

            // ĐỒNG BỘ: Reset số xu về 0 khi người chơi bắt đầu ván mới từ Menu
            gameView.currentCoins = 0
        }
    }

    // --- MÀN HÌNH GAME ---
    GameView {
        id: gameView
        anchors.fill: parent
        visible: false // Mới mở app lên thì Game ẩn

        // BẮT TÍN HIỆU ĐƯỢC PHÁT RA TỪ NÚT EXIT Ở BƯỚC TRƯỚC
        onReturnToMenu: {
            gameView.visible = false // Ẩn Game
            menuView.visible = true  // Hiện Menu lại
        }
        onRestartRequested: {
            bgMusic.play()
        }
        onShieldActivated: {
            shieldActivateSound.play()
        }

        onShieldDeactivated: {
            shieldEndSound.play()
            // Trả âm lượng nhạc nền về bình thường
            bgMusic.audioOutput.volume = 0.7
        }
    }

    // ==========================================
    // 🎵 HỆ THỐNG ÂM THANH (AUDIO SYSTEM)
    // ==========================================
    MediaPlayer {
        id: bgMusic

        // 🌟 1. KHAI BÁO PLAYLIST: Fen thêm các bài nhạc của fen vào mảng này nhé
        property var bgPlaylist: [
            "sounds/game.wav",
             "sounds/game2.mp3"
        ]
        property int currentMusicIndex: 0 // Chỉ số bài nhạc đang phát

        function playRandomMusic() {
            currentMusicIndex = Math.floor(Math.random() * bgPlaylist.length);
            source = bgPlaylist[currentMusicIndex];
            play();
        }

        // Lấy nguồn nhạc động dựa vào chỉ số hiện tại
        source: bgPlaylist[currentMusicIndex]

        audioOutput: AudioOutput {
            id: bgAudioOutput
            volume: 0.7
        }

        // 🌟 2. SỬA TẠI ĐÂY: Để bằng 1 (phát hết bài thì dừng) để hệ thống nhận biết được EndOfMedia
        loops: 1

        // 🌟 3. TỰ ĐỘNG ĐỔI BÀI KHI HẾT NHẠC
        onMediaStatusChanged: {
            if (mediaStatus === MediaPlayer.EndOfMedia) {
                console.log("Hết bài rồi, đang xào bài ngẫu nhiên...")

                // Kiểm tra nếu danh sách có từ 2 bài trở lên thì mới cần né bài trùng
                if (bgPlaylist.length > 1) {
                    let nextIndex = currentMusicIndex;

                    // Vòng lặp này chạy cho đến khi tìm được một con số khác số cũ
                    while (nextIndex === currentMusicIndex) {
                        nextIndex = Math.floor(Math.random() * bgPlaylist.length);
                    }
                    currentMusicIndex = nextIndex;
                } else {
                    currentMusicIndex = 0;
                }

                // Nạp bài mới đã được random và quẩy tiếp
                source = bgPlaylist[currentMusicIndex]
                play()
            }
        }
    }

    MediaPlayer {
        id: coinSound
        source: "sounds/coin.mp3"
        audioOutput: AudioOutput {
            volume: 1.0
        }
    }

    SoundEffect {
        id: crashSound
        source: "sounds/crash.wav" // SoundEffect xài .wav là chuẩn bài!
        volume: 1.0
    }

    // Chuyển mp3 sang MediaPlayer, xóa thẻ Audio dư thừa
    MediaPlayer {
        id: shieldActivateSound
        source: "sounds/up.mp3"
        audioOutput: AudioOutput {
            volume: 1.0
        }
    }

    // Chuyển mp3 sang MediaPlayer
    MediaPlayer {
        id: shieldEndSound
        source: "sounds/down.mp3"
        audioOutput: AudioOutput {
            volume: 1.0
        }
    }

    // --- BẮT TÍN HIỆU TỪ C++ ĐỂ PHÁT NHẠC ---
    Connections {
        target: gameEngine

        function onCoinCollected() {
            coinSound.play()
        }

        function onCrashed() {
            // Chỉ phát tiếng nổ và dừng nhạc nền nếu xe KHÔNG trong trạng thái bật khiên
            if (!gameView.isShieldActive) {
                crashSound.play()
                bgMusic.stop()
            }
        }
    }
}