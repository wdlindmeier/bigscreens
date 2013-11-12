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

namespace bigscreens
{
	
const double kGravity			   = 9.81;
	
typedef std::shared_ptr<class TankShot> TankShotRef;

class TankShot
{
    
public:
    
    TankShot();        
    TankShot(float angleRads,
             float yRotationRads,
             float velocity,
             const ci::Vec3f & initialPosition,
             const ci::gl::GlslProgRef & shader,
             const int parentContentID);
    
    ~TankShot(){};
    
    float           getVelocity();
    float           getThetaRads();
    float           getProgress();
    int             getContentID();
    void            update(float amount);
    bool            isDead();
    
    ci::Vec2f       currentProgress();
    void            render();
    void            renderExplosion(ci::CameraPersp & cam);
    void            renderMuzzleFlare(ci::CameraPersp & cam);
    void            renderLine();

private:
    
    ci::Vec2f       progressAt(float amount);
    void            generateLine(const ci::gl::GlslProgRef & shader);
    
    float           mVelocity;
    float           mThetaRads;
    float           mProgress;
    float           mMaxProgress;
    bool            mIsDead;
    bool            mHasExploded;
    float           mExplosionScale;
    float           mYRotationRads;

    ci::Vec2f       mCurrentOffset;
    ci::Vec3f       mInitialPosition;
    ci::Vec3f       mCurrentPosition;
    ci::Vec3f       mPointOfExplosion;
    
    ci::gl::VaoRef  mLineVao;
    ci::gl::VboRef  mLineVbo;
    
    int             mContentID;
    ci::Quatf       mShotQuat;
};
    
}