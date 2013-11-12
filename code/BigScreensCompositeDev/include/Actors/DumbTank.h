//
//  DumbTank.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/10/13.
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
    typedef std::shared_ptr<class DumbTank> DumbTankRef;
	
    class DumbTank
    {
        
    public:
        
        DumbTank() { load(); }
        ~DumbTank(){};
        
        void load();
        void render(ci::CameraPersp & cam, const float alpha = 1.0);
        ObjModelRef & getModel();
        
    protected:
        
        void loadShader();
        void loadModels();
        
        ObjModelRef    mTankModel;

        ci::gl::GlslProgRef mTankShader;
        
    };
	
}