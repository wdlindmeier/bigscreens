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
#include "cinder/gl/Fbo.h"
#include "SharedTypes.hpp"

class TankContent : public bigscreens::RenderableContent
{
    
public:
    
    TankContent(){};
    ~TankContent(){};
    
    void load(const std::string & objFilename);
    void update();
    void render();
    ci::gl::Texture getTexture();
    ci::Vec2i getSize();
    
private:
    
    ci::gl::VboMesh		mVBO;
    ci::TriMesh			mMesh;
    ci::gl::Fbo         mFBO;
    ci::Matrix44f		mRotation;
};