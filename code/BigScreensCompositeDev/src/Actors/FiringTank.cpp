//
//  FiringTank.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/18/13.
//
//

#include "FiringTank.h"
#include "cinder/gl/Shader.h"

using namespace bigscreens;
using namespace ci;
using namespace ci::app;
using namespace std;

FiringTank::FiringTank() :
mBarrelAngleDeg(0)
,mHeadRotationDeg(0)
{
    
}

void FiringTank::update(long progressCounter)
{
    vector<TankShot> keepShots;
    for (TankShot & shot : mShotsFired)
    {
        // Make this absolute, not relative?
        // Maybe this is fine if it's always being updated on the frame.
        shot.update(0.2f);
        if (!shot.isDead())
        {
            keepShots.push_back(shot);
        }
    }
    mShotsFired = keepShots;
}

void FiringTank::setFrameContentID(const int contentID)
{
    mContentID = contentID;
}

void FiringTank::fire(const PositionOrientation & position,
                      const GroundOrientaion & groundOrientation)
{
    // NOTE: ContentID is reset after Render
    if (mContentID < 0)
    {
        console() << "ERROR: Can't Fire. Tank has a content ID of " << mContentID << std::endl;
        return;
    }
    
    const static float kShotVelocity = 200.0f;
    
    // console() << "Firing w/ velocity " << kShotVelocity << " degrees: " << mBarrelAngleDeg << std::endl;
    
    mShotsFired.push_back(TankShot(position,
                                   groundOrientation,
                                   toRadians(mHeadRotationDeg),
                                   toRadians(mBarrelAngleDeg),
                                   kShotVelocity,
                                   mContentID));
}

void FiringTank::renderShots(ci::CameraPersp & cam, const float alpha)
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
            gl::color(ColorAf(0, 0.5, 1, alpha));
            //gl::color(ColorAf(1, 0, 0, 1.0f));
            shot.renderLine();
            
            gl::color(1,1,1,alpha);
            shot.renderMuzzleFlare(cam);
            shot.renderExplosion(cam);
        }
    }
    
    gl::disableAlphaBlending();
}


