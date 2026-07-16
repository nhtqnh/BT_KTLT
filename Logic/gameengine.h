#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <QObject>
#include <QTimer>
#include <QList>
#include "Model/car.h"
#include "Model/obstacle.h"

class GameEngine : public QObject {
    Q_OBJECT
    // 🌟 ĐÃ SỬA: thay QList<QObject*> obstacles bằng ObstacleModel (QAbstractListModel)
    // -> Repeater trong QML sẽ chỉ thêm/xoá đúng 1 delegate mỗi khi có xe spawn/despawn,
    //    thay vì huỷ + build lại toàn bộ danh sách -> hết giật/lag.
    Q_PROPERTY(QObject* obstacleModel READ getObstacleModel CONSTANT)
    Q_PROPERTY(int playerScore READ getPlayerScore NOTIFY scoreChanged)
    Q_PROPERTY(bool gameOver READ isGameOver NOTIFY gameOverChanged)
    Q_PROPERTY(int scrollSpeed READ getScrollSpeed NOTIFY scrollSpeedChanged)

public:
    explicit GameEngine(QObject *parent = nullptr);
    ~GameEngine();

    Q_INVOKABLE void updatePlayerPosition(float x, float y) {
        if (m_playerCar) {
            m_playerCar->setX(x);
            m_playerCar->setY(y);
        }
    }

    int getScrollSpeed() const { return m_scrollSpeed; }

    Q_INVOKABLE void startGame();
    Q_INVOKABLE void pauseGame();
    Q_INVOKABLE void restartGame();
    Q_INVOKABLE void resumeGame(); // 🌟 Lệnh tiếp tục game

    Car* getPlayerCar() const { return m_playerCar; }
    int getPlayerScore() const { return m_playerScore; }
    bool isGameOver() const { return m_isGameOver; }

    QObject* getObstacleModel() const { return m_obstacleModel; }

signals:
    void scrollSpeedChanged(int newSpeed);
    void scoreChanged(int newScore);
    void gameOverChanged();
    void coinCollected();
    void crashed();

private slots:
    void updateGameTick();

private:
    void spawnEnemy();
    void checkCollisions();

    QTimer *m_gameTimer;
    Car *m_playerCar;
    ObstacleModel *m_obstacleModel;
    int m_scrollSpeed = 6;
    int m_playerScore;
    int m_distanceTraveled;
    bool m_isGameOver;
    int m_spawnCounter;
    int m_dynamicSpawnLimit;
};

#endif // GAMEENGINE_H