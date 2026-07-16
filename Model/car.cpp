#include "car.h"

Car::Car(float x, float y, float width, float height)
    : Entity(x, y, width, height),
    m_currentLane(2),
    m_speed(5),
    m_health(100)
{
}

void Car::setLane(int laneIndex) {
    m_currentLane = laneIndex;
}

int Car::getLane() const {
    return m_currentLane;
}

void Car::setSpeed(int speed) {
    m_speed = speed;
}