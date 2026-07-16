#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QObject>
#include <QString>
#include <QAbstractListModel>
#include <QList>
#include <QPointF>
#include "entity.h"

class Obstacle : public QObject, public Entity {
    Q_OBJECT

    // 🌟 THÊM 2 DÒNG NÀY: Cấp quyền cho QML tự động lắng nghe và cập nhật tọa độ liên tục
    Q_PROPERTY(float x READ getX CONSTANT)
    Q_PROPERTY(float y READ getY NOTIFY yChanged)

public:
    enum Type { car1, Car2, Car3, Car4, Barrier, Barrier1, Coin };

    Obstacle(float x, float y, float width, float height, Type type, QObject *parent = nullptr);

    // 🌟 THÊM 2 HÀM LẤY GIÁ TRỊ NHANH:
    float getX() const { return getPosition().x(); }
    float getY() const { return getPosition().y(); }

    Q_INVOKABLE float getWidth() const;
    Q_INVOKABLE float getHeight() const;
    Q_INVOKABLE QString imagePath() const;
    Q_INVOKABLE QPointF getPosition() const;

    void setLane(int laneIndex);
    int getLane() const;
    void setSpeed(int speed);
    int getSpeed() const;
    Type getType() const;

    void updatePosition(int speed) override;

signals:
    void yChanged();

private:
    int m_lane;
    int m_speed;
    Type m_type;
};
// 🌟 THAY THẾ CHO Q_PROPERTY(QList<QObject*> obstacles ...) CŨ TRONG GameEngine
//
// Lý do: QList<QObject*> làm Q_PROPERTY khiến QML/Repeater không biết chính xác
// PHẦN TỬ NÀO thay đổi mỗi khi NOTIFY (obstaclesChanged) phát ra -> Repeater phải
// huỷ + tạo lại TOÀN BỘ delegate mỗi lần 1 xe spawn hoặc bị xoá.
// Đây chính là nguyên nhân giật/lag và cảm giác "vật cản không xuất hiện".
//
// QAbstractListModel + beginInsertRows()/beginRemoveRows() báo cho QML biết
// CHÍNH XÁC 1 dòng nào thêm/xoá -> Repeater chỉ tạo/huỷ đúng 1 delegate đó,
// các delegate khác giữ nguyên, không bị rebuild lại.

class ObstacleModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { ObstacleRole = Qt::UserRole + 1 };

    explicit ObstacleModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex & = QModelIndex()) const override
    {
        return m_list.size();
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid() || index.row() >= m_list.size())
            return {};
        if (role == ObstacleRole)
            return QVariant::fromValue<QObject*>(m_list.at(index.row()));
        return {};
    }

    QHash<int, QByteArray> roleNames() const override
    {
        return { { ObstacleRole, "obstacle" } };
    }

    // Thêm 1 vật cản mới -> chỉ báo QML thêm đúng 1 dòng
    void addObstacle(Obstacle *obs)
    {
        beginInsertRows(QModelIndex(), m_list.size(), m_list.size());
        m_list.append(obs);
        endInsertRows();
    }

    // Xoá 1 vật cản theo index -> chỉ báo QML xoá đúng 1 dòng, tự delete luôn
    void removeAt(int row)
    {
        if (row < 0 || row >= m_list.size())
            return;
        beginRemoveRows(QModelIndex(), row, row);
        Obstacle *obs = m_list.takeAt(row);
        delete obs;
        endRemoveRows();
    }

    // Dùng khi startGame() / chơi lại: xoá sạch toàn bộ vật cản cũ
    void clearAll()
    {
        if (m_list.isEmpty())
            return;
        beginResetModel();
        qDeleteAll(m_list);
        m_list.clear();
        endResetModel();
    }

    Obstacle* at(int row) const { return m_list.value(row); }
    int count() const { return m_list.size(); }

private:
    QList<Obstacle*> m_list;
};

#endif // OBSTACLEMODEL_H