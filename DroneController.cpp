#include "DroneController.h"
#include <glm/glm.hpp>
#include <cmath>

DroneController::DroneController(DroneModel& model)
    : mModel(model)
    , mPropSpeed(180.0f)
    , mRollSpeed(180.0f)
    , mIsRolling(false)
    , mRollAngleAccum(0.0f)
{
    // match your initial values if needed
}

void DroneController::increasePropSpeed(float delta)
{
    mPropSpeed += delta;
    if(mPropSpeed < 0.f) mPropSpeed = 0.f;
}

void DroneController::decreasePropSpeed(float delta)
{
    mPropSpeed -= delta;
    if(mPropSpeed < 0.f) mPropSpeed = 0.f;
}

void DroneController::updatePropAngle(float dt)
{
    float angle = mModel.getPropAngle();
    angle += mPropSpeed * dt;
    if(angle >= 360.f)
        angle = fmod(angle, 360.f);
    mModel.setPropAngle(angle);
}

void DroneController::startRoll()
{
    if(!mIsRolling)
    {
        mIsRolling = true;
        mRollAngleAccum = 0.f; // reset the roll progress
        mModel.setRollAngle(0.f);
    }
}

void DroneController::updateRoll(float dt)
{
    if(mIsRolling)
    {
        float rollAngle = mModel.getRollAngle();
        rollAngle += mRollSpeed * dt;
        mRollAngleAccum += mRollSpeed * dt;

        if(mRollAngleAccum >= 360.f)
        {
            // complete roll
            rollAngle = 0.f;
            mIsRolling = false;
            mRollAngleAccum = 0.f;
        }
        mModel.setRollAngle(rollAngle);
    }
}

void DroneController::turnYaw(float amount)
{
    float yaw = mModel.getYaw();
    yaw += amount;
    mModel.setYaw(yaw);
}

void DroneController::turnPitch(float amount)
{
    float pitch = mModel.getPitch();
    pitch += amount;
    mModel.setPitch(pitch);
}

void DroneController::moveForward(float dist)
{
    // Build forward vector from yaw/pitch
    glm::vec3 baseForward(0.f, 0.f, 1.f);

    float yawRad   = glm::radians(mModel.getYaw());
    float pitchRad = glm::radians(mModel.getPitch());

    glm::vec3 forward;
    forward.x = baseForward.x * cos(yawRad) - baseForward.z * sin(yawRad);
    forward.y = sin(pitchRad);
    forward.z = baseForward.z * cos(yawRad) + baseForward.x * sin(yawRad);

    forward = glm::normalize(forward);

    glm::vec3 pos = mModel.getPosition();
    pos += forward * dist;
    mModel.setPosition(pos);
}

void DroneController::moveBackward(float dist)
{
    moveForward(-dist);
}

void DroneController::reset()
{
    // Reset the model’s position/orientation
    mModel.setPosition(glm::vec3(0.f, 1.f, 0.f));
    mModel.setYaw(45.f);
    mModel.setPitch(0.f);
    mModel.setRollAngle(0.f);
    mModel.setPropAngle(0.f);

    // Reset controller state
    mPropSpeed = 180.f;
    mIsRolling = false;
    mRollAngleAccum = 0.f;
}
