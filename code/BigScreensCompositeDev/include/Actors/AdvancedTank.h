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
#include "FiringTank.h"

#pragma once

namespace bigscreens
{
	
const static float kDefaultTankWheelSpeedMulti = 6.0f;
    
typedef std::shared_ptr<class AdvancedTank> AdvancedTankRef;
	
class AdvancedTank : public FiringTank
{
    
public:
    
    AdvancedTank();
    ~AdvancedTank(){};

    void update(long progressCounter);
    void render(const float alpha = 1.0);
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
    
    //float           mBarrelAngleDeg;
    //float           mHeadRotationDeg;
    float           mWheelRotation;
    float           mGearRotation;
    float           mShotProgress;
    float           mWheelProgressMulti;
    ci::gl::GlslProgRef mTankShader;
    ci::Vec3f       mTargetPosition;
};
	
}