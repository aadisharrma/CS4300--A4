#include "DroneModel.h"

DroneModel::DroneModel()
    : mPropAngle(0.0f)
    , mRollAngle(0.0f)
    , mYaw(45.0f)   // match your initial yaw
    , mPitch(0.0f)
    , mPosition(0.0f, 1.0f, 0.0f) // match your initial position
{
    // Initialize to your preferred defaults
}
