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

    void fire();
    void update(long progressCounter);
    // NOTE: Set a tmp content ID before rendering
    // and before firing a shot. All shots are tied to
    // a specific content ID.
    // Content ID is cleared after the tank has been rendered.
    void setFrameContentID(const int contentID);
    void render(ci::CameraPersp & cam, const float alpha = 1.0);
    void setWheelSpeedMultiplier(const float wheelMultiplier);
    
protected:
    
    void loadShader();
    void loadModels();

    ObjModelRef     mBodyModel;
    ObjModelRef     mHeadModel;
    ObjModelRef     mBarrelModel;
    ObjModelRef     mGearWheelModel;
    ObjModelRef     mWheelModel;
    
    std::vector<TankShot>  mShotsFired;
    float           mBarrelAngle;
    float           mHeadRotation;
    float           mWheelRotation;
    float           mGearRotation;
    float           mShotProgress;
    float           mWheelProgressMulti;
    ci::gl::GlslProgRef mTankShader;
    int             mContentID;
    
};
	
}