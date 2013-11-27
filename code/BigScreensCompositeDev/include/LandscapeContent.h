//
//  LandscapeContent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/27/13.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "SharedTypes.hpp"
#include "cinder/TriMesh.h"

namespace bigscreens
{
    class LandscapeContent : public bigscreens::RenderableContent
    {
        
    public:
        
        LandscapeContent();
        ~LandscapeContent(){};
        
        virtual void load(const std::string & textureName);
        void setInitialOffset(const ci::Vec2i & offset);
        void setScrollVector(const ci::Vec2f & vector);
        virtual void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
        ci::CameraPersp& getCamera() { return mCam; }
        
    protected:
        
        ci::gl::TextureRef  mTexture;
        ci::CameraPersp mCam;
        ci::Vec2f mInitialOffset;
        ci::Vec2f mScrollVector;
        
    };
}