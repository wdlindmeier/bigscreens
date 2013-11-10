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

#include "AdvancedTank.h"
#include "FloorPlane.h"

namespace bigscreens {
	
typedef std::shared_ptr<class TankContent> TankContentRef;
    
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
            // void loadObj(const std::string & filename);
    
    virtual void drawScreen(const ci::Rectf & contentRect);
    virtual void drawGround();
    virtual void drawTank();
    
    ci::CameraPersp		mCam;

    ci::Vec3f           mTankPosition;

	AdvancedTankRef		mTank;
    
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
	    
	// FloorPlaneRef		mGroundPlane;
};
}