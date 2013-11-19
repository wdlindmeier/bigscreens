//
//  FiringTank.hpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/18/13.
//
//

#pragma once

#include "cinder/gl/gl.h"
#include "SharedTypes.hpp"
#include "ObjModel.h"
#include "TankShot.h"
#include "SharedTypes.hpp"

namespace bigscreens
{
    
typedef std::shared_ptr<class FiringTank> FiringTankRef;

class FiringTank
{

public:
    
    FiringTank();
    virtual ~FiringTank(){}
  
    void fire(const PositionOrientation & position,
              const GroundOrientaion & groundOrientation);
    void setFrameContentID(const int contentID);
    void update(long progressCounter);
    void renderShots(ci::CameraPersp & cam, const float alpha = 1.0);

protected:
    
    std::vector<TankShot>  mShotsFired;
    int             mContentID;
    float           mBarrelAngleDeg;
    float           mHeadRotationDeg;

};

}