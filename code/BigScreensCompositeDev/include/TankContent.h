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
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "SharedTypes.hpp"
#include "cinder/TriMesh.h"
#include "GroundContent.h"

namespace bigscreens {
    
    class TankContent : public bigscreens::RenderableContent
    {
        
    public:
        
        TankContent();
        virtual ~TankContent(){};
        
        void setGroundIsVisible(bool isVisible);
        void setGroundOffset(const ci::Vec2f offset);
        void setTankPosition(const ci::Vec3f tankPosition);
        ci::Vec3f getTankPosition();
        ci::CameraPersp& getCamera() { return mCam; }
        
        virtual void load(const std::string & objFilename);
        virtual void update(std::function<void (ci::CameraPersp & cam)> update_func);
        virtual void resetPositions();
        virtual void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
        virtual void reset();
        
    protected:
        
        virtual void loadGround();
        virtual void loadScreen();
        virtual void loadShaders();
                void loadObj(const std::string & filename);
        
        virtual void drawScreen(const ci::Rectf & contentRect);
        virtual void drawGround();
        virtual void drawTank();
        
        ci::CameraPersp		mCam;

        ci::Vec3f           mTankPosition;

        // Tank
        ci::TriMeshRef		mTankMesh;
        ci::gl::VboRef		mTankVbo;
        ci::gl::VboRef		mTankElementVbo;
        ci::gl::VaoRef		mTankVao;
        ci::gl::GlslProgRef mTankShader;
        
        // NOTE: Maybe the screen texture should be up 1 level
        // Screen
        ci::gl::GlslProgRef mTextureShader;
        ci::gl::TextureRef  mScreenTexture;
        ci::gl::VaoRef      mScreenVao;
        ci::gl::VboRef      mScreenVbo;

        // Ground plane
        ci::gl::GlslProgRef mGroundShader;
        ci::gl::TextureRef  mGridTexture;
        GroundContent       mGroundContent;
        ci::Vec2f           mGroundOffset;
        bool                mIsGroundVisible;
  
    };
}