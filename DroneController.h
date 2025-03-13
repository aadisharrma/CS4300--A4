#pragma once

#include "DroneModel.h"

/**
 * DroneController updates the DroneModel based on user input or other logic.
 */
class DroneController
{
public:
    DroneController(DroneModel& model);

    // Example control methods:
    void increasePropSpeed(float delta);
    void decreasePropSpeed(float delta);
    void updatePropAngle(float dt);

    // For rolling:
    void startRoll();
    void updateRoll(float dt);

    // Yaw/pitch updates
    void turnYaw(float amount);
    void turnPitch(float amount);

    // Movement
    void moveForward(float dist);
    void moveBackward(float dist);

    // Reset
    void reset();

    // Accessors for roll state
    bool  isRolling() const   { return mIsRolling; }
    float getPropSpeed() const { return mPropSpeed; }
    float getRollSpeed() const { return mRollSpeed; }

private:
    DroneModel& mModel;

    // We can store "speed" or "rolling" flags in the controller
    float mPropSpeed;   // deg/sec
    float mRollSpeed;   // deg/sec
    bool  mIsRolling;
    float mRollAngleAccum; // track roll progress
};
