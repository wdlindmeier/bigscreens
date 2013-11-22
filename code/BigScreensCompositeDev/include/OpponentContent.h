//
//  OpponentConent.h
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
#include "OpponentGeometry.h"
#include "cinder/Camera.h"
#include "Opponent.h"

namespace bigscreens
{
    typedef std::shared_ptr<class OpponentContent> OpponentContentRef;
    
    class OpponentContent : public bigscreens::RenderableContent
    {
        
    public:
        
        OpponentContent();
        virtual ~OpponentContent(){};
        virtual void load();
        virtual void update(std::function<void (ci::CameraPersp & cam, OpponentRef & opponent)> update_func);
        virtual void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
		virtual ci::Camera & getCamera();
  
    protected:
        
        virtual void loadShaders();
        virtual void loadScreen();        
        virtual void drawScreen(const ci::Rectf & contentRect);
        virtual void renderOpponent();
        virtual void renderMinions();
        
        ci::CameraPersp mCam;
        OpponentRef mOpponent;
        MinionGeometryRef mMinion;

        // TODO: Dedup
        ci::gl::GlslProgRef mTextureShader;
        ci::gl::TextureRef  mScreenTexture;
        ci::gl::VaoRef      mScreenVao;
        ci::gl::VboRef      mScreenVbo;

    };
}