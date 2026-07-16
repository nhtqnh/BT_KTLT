#ifndef ENTITY_H
#define ENTITY_H

#include <QRectF>  // Thư viện quản lý khung hình chữ nhật dạng số thực của Qt
#include <QPointF> // Thư viện quản lý điểm tọa độ (x, y) dạng số thực của Qt

class Entity {
protected:
    // Dùng protected thay vì private để các lớp con (Car, Obstacle) có thể xài ké các biến này
    float m_x;       // Tọa độ trục X (vị trí ngang của vật thể trên màn hình)
    float m_y;       // Tọa độ trục Y (vị trí dọc của vật thể trên màn hình)
    float m_width;   // Chiều rộng của vật thể (tính bằng pixel)
    float m_height;  // Chiều cao của vật thể (tính bằng pixel)

public:
    // Hàm khởi tạo mặc định cho một thực thể (nếu không truyền gì thì tự động bằng 0)
    Entity(float x = 0, float y = 0, float width = 0, float height = 0);

    // Hàm hủy ảo (virtual destructor) - bắt buộc phải có khi chơi hệ kế thừa OOP để tránh rò rỉ bộ nhớ
    virtual ~Entity() = default;

    // 🌟 ĐÃ THÊM: 2 hàm Set để nhận tọa độ cập nhật liên tục từ QML
    void setX(float newX) { m_x = newX; }
    void setY(float newY) { m_y = newY; }

    // Hàm cập nhật vị trí dịch chuyển theo trục Y khi xe địch hoặc vật cản lao xuống đáy màn hình
    virtual void updatePosition(int dy);

    // Hàm trả về khung hình chữ nhật bao quanh vật thể để thằng làm Logic gọi ra check đụng xe
    QRectF getBoundingBox() const;

    // Hàm lấy tọa độ hiện tại (x, y) để thằng làm Đồ họa (View) biết chỗ mà vẽ hình .png lên
    QPointF getPosition() const;
};

#endif // ENTITY_H