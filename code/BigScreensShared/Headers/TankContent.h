//
//  TankContent.h
//  GridMaker
//
//  Created by William Lindmeier on 10/8/13.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "SharedTypes.hpp"
#include "cinder/TriMesh.h"

namespace bigscreens {

class TankContent : public bigscreens::RenderableContent
{
    
public:
    
    TankContent(){};
    ~TankContent(){};
    
    void load(const std::string & objFilename);
    void update();
    void render();
    void reset();
	ci::CameraPersp& getCamera() { return mCam; }
    
private:
	ci::TriMeshRef		mMesh;
    ci::Matrix44f		mRotation;
	ci::CameraPersp		mCam;
	ci::gl::VboRef		mVbo, mElementVbo;
	ci::gl::VaoRef		mVao;
	ci::gl::GlslProgRef mGlsl;
};

}