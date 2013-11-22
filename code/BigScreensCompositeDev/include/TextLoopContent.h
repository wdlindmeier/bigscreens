//
//  TextLoopContent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/21/13.
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
    enum TextAlign
    {
        TextAlignLeft,
        TextAlignRight
    };
    
    class TextLoopContent : public bigscreens::RenderableContent
    {
        
    public:
        
        TextLoopContent();
        ~TextLoopContent(){};
        
        virtual void load();
        virtual void update();
        virtual void setTextWithTiming(std::vector<std::pair<long, std::string> > & textWithTiming);
        virtual void setAbsoluteLineHeightAndAlignment(const float height, TextAlign align);
        virtual void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
        ci::CameraOrtho & getCamera() { return mCam; }
        virtual bool drawsOutline(){ return false; }
        
    protected:
        
        void setText(const std::string & string);
        
        std::vector<std::pair<long, std::string> > mTextWithTiming;
        ci::gl::TextureRef  mTexture;
        ci::CameraOrtho     mCam;
        ci::Surface         mFontSurf;
        TextAlign           mTextAlign;
        float               mLineHeight;
        std::string         mCurrentString;
    };
}