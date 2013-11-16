//
//  AdvancedTank.cpp
//  AdvancedTank
//
//  Created by William Lindmeier on 11/3/13.
//
//

#include "AdvancedTank.h"
#include "cinder/gl/Shader.h"

using namespace ci;
using std::vector;
using namespace ci::app;
using namespace bigscreens;

AdvancedTank::AdvancedTank() :
mWheelProgressMulti(kDefaultTankWheelSpeedMulti)
,mBarrelAngleDeg(0)
,mWheelRotation(0)
,mGearRotation(0)
,mShotProgress(0)
,mHeadRotationDeg(0)
,mTargetPosition(0,0,0)
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
    ObjModel *bodyModel = new ObjModel();
    bodyModel->load("tank_body.obj", mTankShader);
    mBodyModel = ObjModelRef(bodyModel);
    
    ObjModel *headModel = new ObjModel();
    headModel->load("tank_head_centered.obj", mTankShader);
    mHeadModel = ObjModelRef(headModel);
    
    ObjModel *barrelModel = new ObjModel();
    barrelModel->load("tank_barrel_centered.obj", mTankShader);
    mBarrelModel = ObjModelRef(barrelModel);
    
    ObjModel *gearWheelModel = new ObjModel();
    gearWheelModel->load("tank_gear_wheel_centered.obj", mTankShader);
    mGearWheelModel = ObjModelRef(gearWheelModel);
 
    ObjModel *wheelModel = new ObjModel();
    wheelModel->load("tank_wheel_centered.obj", mTankShader);
    mWheelModel = ObjModelRef(wheelModel);
}

void AdvancedTank::setWheelSpeedMultiplier(const float wheelMultiplier)
{
    mWheelProgressMulti = wheelMultiplier;
}

void AdvancedTank::setTargetPosition(const ci::Vec3f & targetPos)
{
    mTargetPosition = targetPos;
}

void AdvancedTank::setFrameContentID(const int contentID)
{
    mContentID = contentID;
}

void AdvancedTank::fire(const PositionOrientation & position,
                        const GroundOrientaion & groundOrientation)
{
    // NOTE: ContentID is reset after Render
    if (mContentID < 0)
    {
        console() << "ERROR: Can't Fire. Tank has a content ID of " << mContentID << std::endl;
        return;
    }
    
    const static float kShotVelocity = 200.0f;
    
    console() << "Firing w/ velocity " << kShotVelocity << " degrees: " << mBarrelAngleDeg << std::endl;

    mShotsFired.push_back(TankShot(position,
                                   groundOrientation,
                                   toRadians(mHeadRotationDeg),
                                   toRadians(mBarrelAngleDeg),
                                   kShotVelocity,
                                   mTankShader,
                                   mContentID));
    
}

void AdvancedTank::update(long progressCounter)
{
    mWheelRotation = fmod(progressCounter * mWheelProgressMulti, 360.0f);
    mGearRotation = mWheelRotation * 1.25f; // gear is smaller ∴ faster
    
    // Aim the head at the target
    float radsTarget = atan2f(mTargetPosition.x, mTargetPosition.z);

    // TODO: Make the angle more intentional
    mBarrelAngleDeg = (10.0f + (((1.0 + sin(progressCounter * 0.01)) * 0.5) * 40.0f)) * -1;
    
    mHeadRotationDeg = toDegrees(radsTarget);

    vector<TankShot> keepTanks;
    for (TankShot & shot : mShotsFired)
    {
        // Make this absolute, not relative?
        // Maybe this is fine if it's always being updated on the frame.
        shot.update(0.2f);
        if (!shot.isDead())
        {
            keepTanks.push_back(shot);
        }
    }
    mShotsFired = keepTanks;
}

void AdvancedTank::render(const float alpha)
{
    mTankShader->bind();

    gl::setDefaultShaderVars();
    
    // NOTE: There are so many lines, we throttle the alpha to 0.25 max
    mTankShader->uniform("uColor", ColorAf( 1, 1, 1, 0.5 * alpha ));

    mBodyModel->render();

    // Head (rotates)
    gl::pushMatrices();
    gl::rotate(mHeadRotationDeg, 0, 1, 0);
    mHeadModel->render();
    
        // Barrel
        gl::pushMatrices();
        gl::translate(Vec3f(0.0f, kTankBarrelOffsetY, 0.0f));
        gl::translate(Vec3f(0, 0, kTankHeadOffsetZ));
        gl::rotate(mBarrelAngleDeg, 1, 0, 0);
        mBarrelModel->render();
        gl::popMatrices();
    
    gl::popMatrices();

    // Gear Wheel 1
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 79.86f, 222.193f));
    gl::rotate(mGearRotation
               , 1, 0, 0);
    mGearWheelModel->render();
    gl::popMatrices();

    // Gear Wheel 2
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 79.86f, -339.32f));
    gl::rotate(mGearRotation, 1, 0, 0);
    mGearWheelModel->render();
    gl::popMatrices();
    
    // Wheel 1
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, 151.78f));
    gl::rotate(mWheelRotation, 1, 0, 0);
    mWheelModel->render();
    gl::popMatrices();

    // Wheel 2
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, 68.739f));
    gl::rotate(mWheelRotation, 1, 0, 0);
    mWheelModel->render();
    gl::popMatrices();

    // Wheel 3
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, -14.03f));
    gl::rotate(mWheelRotation, 1, 0, 0);
    mWheelModel->render();
    gl::popMatrices();

    // Wheel 4
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, -97.15f));
    gl::rotate(mWheelRotation, 1, 0, 0);
    mWheelModel->render();
    gl::popMatrices();
    
    // Wheel 5
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, -180.04f));
    gl::rotate(mWheelRotation, 1, 0, 0);
    mWheelModel->render();
    gl::popMatrices();
    
    // Wheel 6
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, -262.72f));
    gl::rotate(mWheelRotation, 1, 0, 0);
    mWheelModel->render();
    gl::popMatrices();
    
    mTankShader->unbind();
    
}

void AdvancedTank::renderShots(ci::CameraPersp & cam, const float alpha)
{
    // Draw the tip of the barrel for reference.
    // This can be the shot bloom.

    gl::bindStockShader(gl::ShaderDef().color());
    gl::enableAdditiveBlending();

    // Draw the shot lines
    gl::disableDepthRead();
    gl::disableDepthWrite();
    for (TankShot & shot : mShotsFired)
    {
        if (shot.getContentID() == mContentID)
        {
            //gl::color(ColorAf(0.8, 1, 1, 0.5f * alpha));
            gl::color(ColorAf(1, 0, 0, 1.0f));
            shot.renderLine();
            
            gl::color(1,1,1,alpha);
            shot.renderMuzzleFlare(cam);
            shot.renderExplosion(cam);
        }
    }

    gl::disableAlphaBlending();
}
