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
    class AdvancedTank
    {
        
    public:
        
        AdvancedTank() {};
        virtual ~AdvancedTank(){};
        
        void load();
        void fire();
        virtual void render(long progressCounter);
        
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
        float       mShotProgress;
        
        ci::gl::GlslProgRef mTankShader;
        
    };
}