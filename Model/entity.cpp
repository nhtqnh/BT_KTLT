#include "entity.h"

    Entity::Entity(float x, float y, float width, float height)
    : m_x(x), m_y(y), m_width(width), m_height(height) {}

void Entity::updatePosition(int dy) {
    m_y += dy; // Vì trục Y của Qt tăng dần từ trên xuống, cộng thêm dy nghĩa là vật thể đang rơi xuống
}

QRectF Entity::getBoundingBox() const
{
    return QRectF(m_x + 15, m_y + 15,
        m_width - 30, m_height - 30);
}

QPointF Entity::getPosition() const {
    // Trả về một điểm tọa độ chứa cặp số (x, y)
    return QPointF(m_x, m_y);
}