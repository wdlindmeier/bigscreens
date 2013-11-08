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
	
typedef std::shared_ptr<class AdvancedTank> AdvancedTankRef;
	
class AdvancedTank
{
    
public:
    
    AdvancedTank() { load(); }
    ~AdvancedTank(){};
    
    void load();
    void fire();
    void update(long progressCounter);
    void render(ci::CameraPersp & cam);
    
protected:
    
    void loadShader();
    void loadModels();

    ObjModel    mBodyModel;
    ObjModel    mHeadModel;
    ObjModel    mBarrelModel;
    ObjModel    mGearWheelModel;
    ObjModel    mWheelModel;
    
    std::vector<TankShot>  mShotsFired;
    float       mBarrelAngle;
    float       mWheelRotation;
    float       mGearRotation;
    float       mShotProgress;
    
    ci::gl::GlslProgRef mTankShader;
    
};
	
}