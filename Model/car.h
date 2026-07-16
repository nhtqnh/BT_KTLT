#ifndef CAR_H
#define CAR_H

#include "entity.h"

class Car : public Entity {
public:
    // Hàm khởi tạo nhận đầy đủ 4 tham số truyền cho Entity, kết hợp nạp giá trị mặc định
    Car(float x, float y, float width, float height);

    // ĐÁP ỨNG ĐỦ DANH SÁCH API THEO YÊU CẦU CỦA FILE TÀI LIỆU
    void setLane(int laneIndex);  // Đặt thực thể vào làn đường chỉ định (1, 2 hoặc 3) [cite: 23]
    int getLane() const;          // Lấy làn đường hiện tại của đối tượng [cite: 24]
    void setSpeed(int speed);     // Định cấu hình vận tốc rơi/di chuyển của xe cản địa [cite: 25]

private:
    int m_currentLane;            // Biến lưu làn đường hiện tại (1, 2) [cite: 38]
    int m_speed;                  // Vận tốc di chuyển của xe [cite: 6]
    int m_health;                 // Chỉ số máu của xe (Mặc định = 100) [cite: 6]
};

#endif // CAR_H