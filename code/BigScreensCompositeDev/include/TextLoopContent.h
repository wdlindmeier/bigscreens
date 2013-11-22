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
#include "GridLayout.h"
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
        void setTextForContentID(const TextTimeline & textWithTime,
                                 const int contentID,
                                 const float absoluteLineHeight);
        bool hasTextForContentID(const int contentID);
        virtual void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
        void newLayoutWasSet(const GridLayout & currentLayout);
        ci::CameraOrtho & getCamera() { return mCam; }
        virtual bool drawsOutline(){ return false; }
        
    protected:
        
        ci::gl::TextureRef textureForString(const std::string & str, const float scale);
        ci::gl::TextureRef currentContentFrame();
        ci::CameraOrtho     mCam;
        ci::Surface         mFontSurf;
        std::map<int, std::vector<ci::gl::TextureRef> > mTextures;
        std::map<int, std::vector<int> > mTextureDurations;
        std::map<int, int > mContentDurations;
    };
}