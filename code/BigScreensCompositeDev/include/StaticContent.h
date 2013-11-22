//
//  StaticContent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/22/13.
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
    typedef std::shared_ptr<class StaticContent> StaticContentRef;
    
    class StaticContent : public bigscreens::RenderableContent
    {
        
    public:
        
        StaticContent();
        ~StaticContent(){};
        
        virtual void load();
        virtual void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
        ci::CameraOrtho & getCamera() { return mCam; }
        
    protected:
        
        std::vector<ci::gl::TextureRef>  mTextures;
        ci::CameraOrtho     mCam;
        
    };
}