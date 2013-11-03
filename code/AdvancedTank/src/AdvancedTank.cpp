//
//  AdvancedTank.cpp
//  AdvancedTank
//
//  Created by William Lindmeier on 11/3/13.
//
//

#include "AdvancedTank.h"

using namespace ci;
using std::vector;
using namespace ci::app;
using namespace bigscreens;

void AdvancedTank::load()
{
    loadShader();
    loadModels();
}

void AdvancedTank::loadShader()
{
    gl::GlslProg::Format mFormat;
    mFormat.vertex( loadResource( "basic.vert" ) )
    .fragment( loadResource( "basic.frag" ) );
    mTankShader = gl::GlslProg::create( mFormat );
    mTankShader->bind();
}

void AdvancedTank::loadModels()
{
    mBodyModel.load("tank_body.obj", mTankShader);
    mHeadModel.load("tank_head.obj", mTankShader);
    mBarrelModel.load("tank_barrel_centered.obj", mTankShader);
    mGearWheelModel.load("tank_gear_wheel_centered.obj", mTankShader);
    mWheelModel.load("tank_wheel_centered.obj", mTankShader);
}

void AdvancedTank::fire()
{
    float targetVelocity = 200.0f;
    console() << "Firing w/ velocity " << targetVelocity << " degrees: " << toDegrees(mBarrelAngle) << std::endl;
    mShotsFired.push_back(TankShot(mBarrelAngle, targetVelocity));
}

void AdvancedTank::render(long progressCounter)
{
    gl::enableAdditiveBlending();
    
    mTankShader->bind();

    gl::setDefaultShaderVars();
    
    mTankShader->uniform("uColor", ColorAf(1,1,1,0.25f));
    
    float wheelRotation = fmod(progressCounter * 1, 360.0f);
    float gearRotation = wheelRotation * 1.5f; // gear is smaller / faster

    // TMP: FPO
    float barrelAngle = ((1.0 + sin(progressCounter * 0.01)) * 0.5) * -35.0f;
    mBarrelAngle = barrelAngle * -1; // ci::toRadians(barrelAngle) * -1;
    
    mBodyModel.render();
    mHeadModel.render();
    
    // Barrel
    gl::pushMatrices();
    const static float kBarrelOffsetY = 170.0f;
    const static float kBarrelLength = 461.0f;
    gl::translate(Vec3f(0.0f, kBarrelOffsetY, 0.0f));
    gl::rotate(barrelAngle, 1, 0, 0);
    mBarrelModel.render();
    gl::popMatrices();
    
    // Gear Wheel 1
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 79.86f, 222.193f));
    gl::rotate(gearRotation, 1, 0, 0);
    mGearWheelModel.render();
    gl::popMatrices();

    // Gear Wheel 2
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 79.86f, -339.32f));
    gl::rotate(gearRotation, 1, 0, 0);
    mGearWheelModel.render();
    gl::popMatrices();
    
    // Wheel 1
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, 151.78f));
    gl::rotate(wheelRotation, 1, 0, 0);
    mWheelModel.render();
    gl::popMatrices();

    // Wheel 2
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, 68.739f));
    gl::rotate(wheelRotation, 1, 0, 0);
    mWheelModel.render();
    gl::popMatrices();

    // Wheel 3
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, -14.03f));
    gl::rotate(wheelRotation, 1, 0, 0);
    mWheelModel.render();
    gl::popMatrices();

    // Wheel 4
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, -97.15f));
    gl::rotate(wheelRotation, 1, 0, 0);
    mWheelModel.render();
    gl::popMatrices();
    
    // Wheel 5
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, -180.04f));
    gl::rotate(wheelRotation, 1, 0, 0);
    mWheelModel.render();
    gl::popMatrices();
    
    // Wheel 6
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, -262.72f));
    gl::rotate(wheelRotation, 1, 0, 0);
    mWheelModel.render();
    gl::popMatrices();
    
    // Draw the tip of the barrel for reference.
    // This can be the shot bloom.
    float offX = cos(toRadians(mBarrelAngle)) * kBarrelLength;
    float offY = sin(toRadians(mBarrelAngle)) * kBarrelLength;
    Vec3f barrelHead(0, kBarrelOffsetY + offY, 0 + offX);
    
    gl::enableAdditiveBlending();
    mTankShader->uniform("uColor", ColorAf(1,1,0,1));
    gl::drawCube(barrelHead, Vec3f(50,50,50));

    vector<TankShot> keepTanks;
    for (TankShot & shot : mShotsFired)
    {
        shot.update(0.2f);

        float shotTheta = toRadians(shot.getTheta());
        offX = cos(shotTheta) * kBarrelLength;
        offY = sin(shotTheta) * kBarrelLength;
        Vec3f exitPoint(0, kBarrelOffsetY + offY, 0 + offX);
        
        mTankShader->uniform("uColor", ColorAf(1,0,0,1));
        shot.render(exitPoint);
        if (!shot.isDead())
        {
            keepTanks.push_back(shot);
        }
    }
    mShotsFired = keepTanks;

    mTankShader->unbind();
}
