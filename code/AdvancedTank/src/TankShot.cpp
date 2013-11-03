//
//  Trajectory.cpp
//  AdvancedTank
//
//  Created by William Lindmeier on 11/3/13.
//
//

#include "TankShot.h"

using namespace ci;
using namespace bigscreens;

float TankShot::getVelocity()
{
    return mVelocity;
}

float TankShot::getTheta()
{
    return mTheta;
}

float TankShot::getProgress()
{
    return mProgress;
}

bool TankShot::isDead()
{
    return mIsDead;
}

void TankShot::update(float amount)
{
    mProgress += amount;
}

void TankShot::render(const ci::Vec3f & initialOffset)
{
    Vec2f misslePosition = currentProgress();
    // TODO: Draw fire in the hole
    // TODO: Draw explosion
    if (misslePosition.x > 0 && misslePosition.y > (initialOffset.y * -1))
    {
        Vec3f missleCenter(0, misslePosition.y, misslePosition.x);
        gl::drawCube(initialOffset + missleCenter, Vec3f(50,50,50));
    }
    else
    {
        mIsDead = true;
    }
}

Vec2f TankShot::currentProgress()
{
    float y = mVelocity*sin(mTheta*pi/180.0)*mProgress - 0.5*kGravity*mProgress*mProgress;  // altitude
    float x = mVelocity*cos(mTheta*M_PI/180.0)*mProgress;  // downrange
    return Vec2f(x,y);
}

