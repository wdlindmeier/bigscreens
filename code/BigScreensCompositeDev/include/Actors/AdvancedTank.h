//
//  AdvancedTank.h
//  AdvancedTank
//
//  Created by William Lindmeier on 11/3/13.
//
//

#include "cinder/gl/gl.h"
#include "cinder/ObjLoader.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"
#include "SharedTypes.hpp"
#include "ObjModel.h"
#include "TankShot.h"

#pragma once

namespace bigscreens
{
	
const static float kDefaultTankWheelSpeedMulti = 6.0f;
    
typedef std::shared_ptr<class AdvancedTank> AdvancedTankRef;
	
class AdvancedTank
{
    
public:
    
    AdvancedTank();
    ~AdvancedTank(){};

    void fire(const ci::Vec3f & worldPosition,
              const GroundOrientaion & groundOrientation);
    void update(long progressCounter);
    void setFrameContentID(const int contentID);
    void render(const float alpha = 1.0);
    void renderShots(ci::CameraPersp & cam, const float alpha = 1.0);
    void setWheelSpeedMultiplier(const float wheelMultiplier);
    void setTargetPosition(const ci::Vec3f & targetPos);
    
protected:
    
    void loadShader();
    void loadModels();

    ObjModelRef     mBodyModel;
    ObjModelRef     mHeadModel;
    ObjModelRef     mBarrelModel;
    ObjModelRef     mGearWheelModel;
    ObjModelRef     mWheelModel;
    
    std::vector<TankShot>  mShotsFired;
    float           mBarrelAngleDeg;
    float           mHeadRotationDeg;
    float           mWheelRotation;
    float           mGearRotation;
    float           mShotProgress;
    float           mWheelProgressMulti;
    ci::gl::GlslProgRef mTankShader;
    int             mContentID;
    ci::Vec3f       mTargetPosition;
};
	
}