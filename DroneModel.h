#pragma once
#include <glm/glm.hpp>

/**
 * DroneModel holds the drone’s state:
 *  - propeller angle
 *  - roll angle
 *  - yaw, pitch
 *  - position
 */
class DroneModel
{
public:
    // Constructors
    DroneModel();

    // Getters
    float getPropAngle() const    { return mPropAngle; }
    float getRollAngle() const    { return mRollAngle; }
    float getYaw() const          { return mYaw; }
    float getPitch() const        { return mPitch; }
    glm::vec3 getPosition() const { return mPosition; }

    // Setters
    void setPropAngle(float angle)       { mPropAngle = angle; }
    void setRollAngle(float angle)       { mRollAngle = angle; }
    void setYaw(float angle)             { mYaw = angle; }
    void setPitch(float angle)           { mPitch = angle; }
    void setPosition(const glm::vec3& p) { mPosition = p; }

private:
    float     mPropAngle;
    float     mRollAngle;
    float     mYaw;
    float     mPitch;
    glm::vec3 mPosition;
};
