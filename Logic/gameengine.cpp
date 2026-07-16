#include "gameengine.h"
#include "Model/obstacle.h"
#include "Model/car.h"
#include "Model/entity.h"
#include <QRandomGenerator>

const float LANE_LEFT_X  = 131;
const float LANE_RIGHT_X = 229; // Tọa độ X của làn bên PHẢI
const float SPAWN_Y_START = -180; // Xuất hiện âm hẳn lên trên để ẩn giấu lúc mới sinh

GameEngine::GameEngine(QObject *parent)
    : QObject(parent),
    m_gameTimer(new QTimer(this)),
    m_playerCar(nullptr),
    m_obstacleModel(new ObstacleModel(this)), // 🌟 ĐÃ SỬA: Khởi tạo model mới quản lý vật cản
    m_playerScore(0),
    m_isGameOver(false),
    m_spawnCounter(0)
{
    m_gameTimer->setTimerType(Qt::PreciseTimer);
    connect(m_gameTimer, &QTimer::timeout, this, &GameEngine::updateGameTick);
}

GameEngine::~GameEngine()
{
    delete m_playerCar;
    m_obstacleModel->clearAll(); // 🌟 ĐÃ SỬA: Giải phóng bộ nhớ thông qua Model
}

void GameEngine::startGame()
{
    if (m_playerCar) delete m_playerCar;
    m_playerCar = new Car(200, 550, 90, 110);

    m_obstacleModel->clearAll(); // 🌟 ĐÃ SỬA: Reset sạch vật cản cũ bằng Model

    m_playerScore = 0;
    m_distanceTraveled = 0;
    m_isGameOver = false;
    m_spawnCounter = 0;
    m_scrollSpeed = 5; // Vận tốc nền ban đầu nhẹ nhàng
    m_dynamicSpawnLimit = 150;

    emit scoreChanged(m_playerScore);
    // Không cần phát tín hiệu obstaclesChanged() nữa vì Model sẽ tự động báo QML vẽ lại cực mượt
    m_gameTimer->start(16);
}

void GameEngine::pauseGame()
{
    if (m_gameTimer->isActive()) {
        m_gameTimer->stop();
    }
}

void GameEngine::resumeGame() {
    // Chỉ cho chạy lại nếu game chưa thua
    if (!m_isGameOver) {
        m_gameTimer->start(); // Bật lại timer sinh vật cản
    }
}

void GameEngine::restartGame()
{
    // 1. Dọn sạch toàn bộ xe và đồng xu cũ đang có trên đường chạy
    if (m_obstacleModel) {
        m_obstacleModel->clearAll();
    }

    // 2. Đặt lại điểm số về bằng 0
    m_playerScore = 0;
    m_scrollSpeed = 5;
    m_distanceTraveled = 0; // Reset quãng đường về 0
    m_spawnCounter = 0;     // Reset luôn bộ đếm thời gian sinh xe
    emit scoreChanged(m_playerScore);
    emit scrollSpeedChanged(m_scrollSpeed); // 🌟 THÊM DÒNG NÀY

    // 3. Đặt lại vị trí xe người chơi về giữa làn đường (ví dụ X=170, Y=500 tùy game của bạn)
    if (m_playerCar) {
        m_playerCar->setX(170);
        m_playerCar->setY(500);
    }

    // 4. Hồi sinh trạng thái Game Over thành Chưa chết
    m_isGameOver = false;
    emit gameOverChanged();

    // 5. Cho Timer chạy lại để kích hoạt game chạy tiếp
    if (m_gameTimer && !m_gameTimer->isActive()) {
        m_gameTimer->start();
    }
}

void GameEngine::updateGameTick()
{
    if (m_isGameOver) return;

    // 1. TỰ ĐỘNG TĂNG QUÃNG ĐƯỜNG VÀ TÍNH ĐỘ KHÓ
    m_distanceTraveled++; // Cứ mỗi tick game là quãng đường tăng lên 1
    int oldSpeed = m_scrollSpeed;
    // Tốc độ khởi điểm: 5. Cứ chạy được 500 tick (khoảng 8 giây) thì tăng tốc 1 lần
    m_scrollSpeed = 5 + (m_distanceTraveled / 500);
    if (m_scrollSpeed > 18) m_scrollSpeed = 18; // Max speed

    if (m_scrollSpeed != oldSpeed) {
        emit scrollSpeedChanged(m_scrollSpeed);
    }

    // Nhịp độ sinh xe nhanh dần theo quãng đường
    m_dynamicSpawnLimit = 150 - (m_distanceTraveled / 40);
    if (m_dynamicSpawnLimit < 60) m_dynamicSpawnLimit = 60;

    // 2. DI CHUYỂN VẬT CẢN (VÀ XÓA KHI TRÔI QUA)
    for (int i = m_obstacleModel->count() - 1; i >= 0; --i)
    {
        Obstacle *obj = m_obstacleModel->at(i);

        // Đồng xu và hàng rào trôi bằng vận tốc đường, xe địch trôi nhanh hơn 1 chút
        int currentSpeed = (obj->getType() == Obstacle::Barrier ||
                            obj->getType() == Obstacle::Barrier1 ||
                            obj->getType() == Obstacle::Coin)
                               ? m_scrollSpeed : m_scrollSpeed + 1;

        // 🌟 DÒNG QUAN TRỌNG: Bắt vật cản trôi xuống
        obj->updatePosition(currentSpeed);

        // Xe hoặc Xu trôi qua khỏi màn hình thì chỉ xóa cho nhẹ máy, KHÔNG CỘNG ĐIỂM
        if (obj->getPosition().y() > 700)
        {
            m_obstacleModel->removeAt(i);
        }
    } // KẾT THÚC VÒNG LẶP FOR (Chỉ có 1 vòng lặp duy nhất)

    // 3. SINH XE / XU
    m_spawnCounter++;
    if (m_spawnCounter >= m_dynamicSpawnLimit)
    {
        spawnEnemy();
        m_spawnCounter = 0;
    }

    // 4. KIỂM TRA VA CHẠM
    checkCollisions();
}

void GameEngine::spawnEnemy()
{
    const float SPAWN_Y_NORMAL = -180;
    const float SPAWN_Y_BACK   = -340;

    // 🌟 KHOẢNG CÁCH KHE HẸP: -180 trừ đi 240px (khoảng hơn 2 thân xe)
    const float SPAWN_Y_ZIGZAG = -420;

    // 1. CHỌN NGẪU NHIÊN LÀN ĐƯỜNG VÀ LÀN ĐỐI DIỆN
    int lane = QRandomGenerator::global()->bounded(1, 3); // 1: Trái, 2: Phải
    int oppositeLane = (lane == 1) ? 2 : 1;               // Lấy làn ngược lại

    float x = (lane == 1) ? LANE_LEFT_X : LANE_RIGHT_X;
    float oppositeX = (oppositeLane == 1) ? LANE_LEFT_X : LANE_RIGHT_X;

    // 2. CHỌN TỈ LỆ RA XE
    int patternRate = QRandomGenerator::global()->bounded(100);

    // TH 1: 20% CƠ HỘI RA 2 XE NỐI ĐUÔI NHAU (CÙNG 1 LÀN)
    if (patternRate < 30)
    {
        Obstacle::Type type1 = static_cast<Obstacle::Type>(QRandomGenerator::global()->bounded(0, 4));
        Obstacle* obs1 = new Obstacle(x, SPAWN_Y_NORMAL, 90, 110, type1, this);
        obs1->setLane(lane);

        Obstacle::Type type2 = static_cast<Obstacle::Type>(QRandomGenerator::global()->bounded(0, 4));
        Obstacle* obs2 = new Obstacle(x, SPAWN_Y_BACK, 90, 110, type2, this);
        obs2->setLane(lane);

        m_obstacleModel->addObstacle(obs1);
        m_obstacleModel->addObstacle(obs2);
    }
    // 🌟 TH 2: 30% CƠ HỘI ZICZAC (1 TRÁI, 1 PHẢI, KHE HẸP ĐỂ LÁCH)
    else if (patternRate >= 30 && patternRate < 50)
    {
        // Xe 1 (Đi trước)
        Obstacle::Type type1 = static_cast<Obstacle::Type>(QRandomGenerator::global()->bounded(0, 4));
        Obstacle* obs1 = new Obstacle(x, SPAWN_Y_NORMAL, 90, 110, type1, this);
        obs1->setLane(lane);

        // Xe 2 (Đi sau ở làn đối diện, khe hẹp sát rạt)
        Obstacle::Type type2 = static_cast<Obstacle::Type>(QRandomGenerator::global()->bounded(0, 4));
        Obstacle* obs2 = new Obstacle(oppositeX, SPAWN_Y_ZIGZAG, 90, 110, type2, this);
        obs2->setLane(oppositeLane);

        m_obstacleModel->addObstacle(obs1);
        m_obstacleModel->addObstacle(obs2);
    }
    // TH 3: 50% CƠ HỘI RA 1 XE LẺ HOẶC ĐỒNG XU
    else
    {
        float width = 90;
        float height = 110;
        Obstacle::Type type;

        // Tỷ lệ 20% ra Đồng Xu, 80% ra Vật cản
        if (QRandomGenerator::global()->bounded(100) < 30)
        {
            type = Obstacle::Coin;
            width = 50;
            height = 50;
        }
        else
        {
            type = static_cast<Obstacle::Type>(QRandomGenerator::global()->bounded(0, 6));

            if (type == Obstacle::Barrier || type == Obstacle::Barrier1) {
                width = 100;
                height = 150;
            }
        }

        Obstacle* obs = new Obstacle(x, SPAWN_Y_NORMAL, width, height, type, this);
        obs->setLane(lane);
        m_obstacleModel->addObstacle(obs);
    }
}

void GameEngine::checkCollisions()
{
    if (!m_playerCar || m_isGameOver) return;

    // 1. Hitbox của xe người chơi (xe to nên thu nhỏ lại tí cho đỡ quá nhạy)
    QRectF playerHitbox = m_playerCar->getBoundingBox().adjusted(5, 5, -5, -5);

    // Vòng lặp ngược từ dưới lên chuẩn chỉ
    for (int i = m_obstacleModel->count() - 1; i >= 0; --i)
    {
        Obstacle* obs = m_obstacleModel->at(i);
        QRectF obsHitbox;

        // 🌟 BƯỚC CHỐT HẠ: PHÂN CHIA HITBOX THEO LOẠI VẬT THỂ
        if (obs->getType() == Obstacle::Coin)
        {
            // Nếu là ĐỒNG XU: Lấy trọn vẹn kích thước gốc (X, Y, Width, Height) không thu nhỏ!
            // Thậm chí bạn có thể mở rộng ra một chút cho người chơi dễ ăn xu
            obsHitbox = QRectF(obs->getX(), obs->getY(), obs->getWidth(), obs->getHeight());
        }
        else
        {
            // Nếu là XE KHÁC / RÀO CHẮN: Lấy bounding box và thu nhỏ lại cho đỡ lấn làn
            obsHitbox = obs->getBoundingBox().adjusted(5, 5, -10, -10);
        }

        // 2. Tiến hành kiểm tra va chạm thực tế
        if (playerHitbox.intersects(obsHitbox))
        {
            // Nếu chạm trúng ĐỒNG XU
            if (obs->getType() == Obstacle::Coin)
            {
                m_playerScore += 10;
                emit scoreChanged(m_playerScore);
                emit coinCollected();             // Phát tiếng ting ting

                m_obstacleModel->removeAt(i);     // Xóa đồng xu bốc hơi khỏi màn hình
            }
            // Nếu chạm trúng XE CHƯỚNG NGẠI VẬT / RÀO CHẮN
            else
            {
                m_isGameOver = true;
                emit gameOverChanged();
                emit crashed();                   // Phát tiếng rầm
                break;
            }
        }
    }
}