//
//  TankShot.h
//  AdvancedTank
//
//  Created by William Lindmeier on 11/3/13.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"
#include "SharedTypes.hpp"

namespace bigscreens
{
	
const double kGravity			   = 9.81;
	
typedef std::shared_ptr<class TankShot> TankShotRef;

class TankShot
{
    
public:
    
    TankShot();
    TankShot(const PositionOrientation & tankPosition,
             const GroundOrientaion & groundOrientation,
             float barrelDirectionRads,
             float barrelAngleRads,
             float velocity,
             const ci::gl::GlslProgRef & shader,
             const int parentContentID);

    ~TankShot(){};

    float           getProgress();
    int             getContentID();
    void            update(float amount);
    bool            isDead();

    void            renderExplosion(ci::CameraPersp & cam);
    void            renderMuzzleFlare(ci::CameraPersp & cam);
    void            renderLine();

private:
    
    void            generateLine(const ci::gl::GlslProgRef & shader);
    
    float           mVelocity;
    float           mProgress;
    float           mMaxProgress;
    bool            mIsDead;
    bool            mHasExploded;
    float           mExplosionScale;

    ci::Vec3f       mPointOfExplosion;
    ci::Vec3f       mShotOrigin;

    ci::Matrix44f   mTankMat;
    ci::Matrix44f   mGunMat;
    
    GroundOrientaion mGroundOrientation;
    PositionOrientation mTankOrientation;
    
    ci::gl::VaoRef  mLineVao;
    ci::gl::VboRef  mLineVbo;
    
    int             mContentID;
};
    
}