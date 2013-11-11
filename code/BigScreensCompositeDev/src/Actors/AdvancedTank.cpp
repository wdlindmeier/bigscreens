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

const static float kBarrelOffsetY = 170.0f;
const static float kBarrelLength = 461.0f;

AdvancedTank::AdvancedTank() :
mWheelProgressMulti(kDefaultTankWheelSpeedMulti)
,mBarrelAngle(0)
,mWheelRotation(0)
,mGearRotation(0)
,mShotProgress(0)
,mHeadRotation(0)
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
    headModel->load("tank_head.obj", mTankShader);
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

void AdvancedTank::setFrameContentID(const int contentID)
{
    mContentID = contentID;
}

void AdvancedTank::fire()
{
    // NOTE: ContentID is reset after Render
    if (mContentID < 0)
    {
        console() << "ERROR: Can't Fire. Tank has a content ID of " << mContentID << std::endl;
        return;
    }
    
    float targetVelocity = 200.0f;
    
    console() << "Firing w/ velocity " << targetVelocity << " degrees: " << toDegrees(mBarrelAngle) << std::endl;
    
    float shotTheta = toRadians(mBarrelAngle);
    float offX = cos(shotTheta) * kBarrelLength;
    float offY = sin(shotTheta) * kBarrelLength;
    Vec3f exitPoint(0, kBarrelOffsetY + offY, 0 + offX);
    
    float yRotRads = toRadians(mHeadRotation);

    mShotsFired.push_back(TankShot(mBarrelAngle,
                                   yRotRads,
                                   targetVelocity,
                                   exitPoint,
                                   mTankShader,
                                   mContentID));
}

void AdvancedTank::update(long progressCounter)
{
    mWheelRotation = fmod(progressCounter * mWheelProgressMulti, 360.0f);
    mGearRotation = mWheelRotation * 1.25f; // gear is smaller ∴ faster
    
    // TMP angles
    mBarrelAngle = ((1.0 + sin(progressCounter * 0.01)) * 0.5) * 35.0f;
    mHeadRotation = (mBarrelAngle * -2) + mBarrelAngle;

    vector<TankShot> keepTanks;
    for (TankShot & shot : mShotsFired)
    {
        // TODO: Make this absolute, not relative?
        // Each tank needs an array of shots
        shot.update(0.2f);
        if (!shot.isDead())
        {
            keepTanks.push_back(shot);
        }
    }
    mShotsFired = keepTanks;
}

void AdvancedTank::render(ci::CameraPersp & cam, const float alpha)
{
    gl::enableAdditiveBlending();
    
    mTankShader->bind();

    gl::setDefaultShaderVars();
    
    // NOTE: There are so many lines, we throttle the alpha to 0.25 max
    mTankShader->uniform("uColor", ColorAf( 1, 1, 1, 0.25 * alpha ));

    mBodyModel->render();

    // Head (rotates)
    gl::pushMatrices();
    gl::rotate(mHeadRotation, 0, 1, 0);
    mHeadModel->render();
    
        // Barrel
        gl::pushMatrices();
        gl::translate(Vec3f(0.0f, kBarrelOffsetY, 0.0f));
        gl::rotate(mBarrelAngle * -1, 1, 0, 0);
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

    // Draw the tip of the barrel for reference.
    // This can be the shot bloom.
    gl::enableAdditiveBlending();

    float offX = cos(toRadians(mBarrelAngle)) * kBarrelLength;
    float offY = sin(toRadians(mBarrelAngle)) * kBarrelLength;
    Vec3f barrelHead(0, kBarrelOffsetY + offY, 0 + offX);
    
    // Draw the shot lines
    
    for (TankShot & shot : mShotsFired)
    {
        if (shot.getContentID() == mContentID)
        {
            mTankShader->uniform("uColor", ColorAf(0.8, 1, 1, 0.5f * alpha));
            shot.renderLine();
            
            mTankShader->uniform("uColor", ColorAf(1, 1, 1, alpha));
            shot.renderMuzzleFlare(cam);
            shot.renderExplosion(cam);
        }
    }
    
    /*
//    // Draw the shot projectiles
//    mTankShader->uniform("uColor", ColorAf(1, 0, 0, alpha));
//    for (TankShot & shot : mShotsFired)
//    {
//        shot.render();
//    }

    // Draw the muzzle flare
    for (TankShot & shot : mShotsFired)
    {
        shot.renderMuzzleFlare(cam);
    }

    // Draw the shot explosions
    for (TankShot & shot : mShotsFired)
    {
        shot.renderExplosion(cam);
    }
    */
    
    mTankShader->unbind();
    
    // Clear out the content ID
    mContentID = -1;

}
