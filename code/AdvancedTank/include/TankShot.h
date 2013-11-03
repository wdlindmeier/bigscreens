//
//  TankShot.h
//  AdvancedTank
//
//  Created by William Lindmeier on 11/3/13.
//
//

#pragma once

#include "cinder/Cinder.h"

namespace bigscreens
{
    const double kGravity			   = 9.81;

    class TankShot
    {
        
    public:
        
        TankShot() :
        mVelocity(0),
        mTheta(0),
        mProgress(0)
        {};
        
        TankShot(float angleRadians, float velocity) :
        mVelocity(velocity),
        mTheta(angleRadians),
        mProgress(0)
        {
        };
        
        ~TankShot(){};
        
        float getVelocity();
        float getTheta();
        float getProgress();
        void  update(float amount);
        bool  isDead();
        
        ci::Vec2f currentProgress();
        void  render(const ci::Vec3f & initialOffset);

    private:
        
        float     mVelocity;
        float     mTheta;
        float     mProgress;
        bool      mIsDead;
    };
    
}