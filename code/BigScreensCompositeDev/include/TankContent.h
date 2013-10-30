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

namespace bigscreens {
    
    class TankContent : public bigscreens::RenderableContent
    {
        
    public:
        
        TankContent();
        virtual ~TankContent(){};
        
        virtual void load(const std::string & objFilename);
        virtual void update();
        virtual void update(std::function<void (ci::CameraPersp & cam)> update_func);
        virtual void render(const ci::Vec2i & screenOffset);
        virtual void reset();
        ci::CameraPersp& getCamera() { return mCam; }
        
    protected:
        
        virtual void loadGround();
        virtual void loadScreen();
        virtual void loadShaders();
        void loadObj(const std::string & filename);
        
        virtual void drawScreen();
        virtual void drawGround();
        virtual void drawTank();
        
        ci::CameraPersp		mCam;


        float               mCameraRotation;
        
        // Tank
        ci::TriMeshRef		mTankMesh;
        ci::gl::VboRef		mTankVbo;
        ci::gl::VboRef		mTankElementVbo;
        ci::gl::VaoRef		mTankVao;
        ci::gl::GlslProgRef mTankShader;
        
        // NOTE: Maybe the screen texture should be up 1 level
        // Screen
        ci::gl::TextureRef  mScreenTexture;
        ci::gl::VaoRef      mScreenVao;
        ci::gl::VboRef      mScreenVbo;

        // Ground plane
        ci::gl::GlslProgRef mTextureShader;
        ci::gl::TextureRef  mGridTexture;
        ci::gl::VaoRef      mGroundVao;
        ci::gl::VboRef      mGroundVbo;
        float               mGroundOffset;
  
    };
}