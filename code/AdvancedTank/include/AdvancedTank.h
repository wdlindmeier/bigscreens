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

#pragma once

namespace bigscreens
{
    class AdvancedTank
    {
        
    public:
        
        AdvancedTank() : mProgress(0) {};
        virtual ~AdvancedTank(){};
        
        void load();
        void update();
        virtual void render(const ci::Vec2f & screenOffset);
        
    protected:
        
        void loadShader();
        void loadModels();

        ObjModel    mBodyModel;
        ObjModel    mHeadModel;
        ObjModel    mBarrelModel;
        ObjModel    mGearWheelModel;
        ObjModel    mWheelModel;
        
        ci::gl::GlslProgRef mTankShader;
        
        long        mProgress;
    };
}