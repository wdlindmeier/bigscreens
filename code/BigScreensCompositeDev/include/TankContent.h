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
//#include "GroundContent.h"
#include "AdvancedTank.h"
#include "OpponentGeometry.h"
#include "FloorPlane.h"
#include "PerlinContent.h"

namespace bigscreens {

typedef std::shared_ptr<class TankContent> TankContentRef;
    
class TankContent : public bigscreens::RenderableContent
{
    
public:
    
    TankContent();
    virtual ~TankContent(){};
    
    void setGroundIsVisible(bool isVisible);
    void setTankPosition(const ci::Vec3f tankPosition);
    ci::Vec3f getTankPosition();
    ci::CameraPersp & getCamera();
    AdvancedTankRef & getTank();
    
    virtual void load();
    virtual void update(std::function<void (ci::CameraPersp & cam, AdvancedTankRef & tank)> update_func);
    virtual void resetPositions();
    virtual void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
    virtual void reset();
    virtual void fireTankGun();
    virtual void setFrameContentID(const int contentID);
    
protected:
    
    // Funcs
    
    virtual void        loadScreen();
    virtual void        loadShaders();
    
    virtual void        drawScreen(const ci::Rectf & contentRect);
    virtual void        drawGround();
    virtual void        drawTank();
    virtual void        renderPositionedTank();
    virtual void        drawTankShots();
    virtual void        drawMinion();
    
    // Vars
    
    ci::CameraPersp		mCam;

    ci::Vec3f           mTankPosition;
    ci::Vec3f           mMinionPosition;

	AdvancedTankRef		mTank;
    MinionGeometryRef   mMinion;

    // Screen
    ci::gl::GlslProgRef mTextureShader;
    ci::gl::TextureRef  mScreenTexture;
    ci::gl::VaoRef      mScreenVao;
    ci::gl::VboRef      mScreenVbo;

    // Ground
    FloorPlaneRef		mGroundPlane;
    bool                mIsGroundVisible;
    
    virtual void        generateGroundMaps();
    virtual void        drawGroundTile(const ci::Vec3i & plot, ci::gl::TextureRef & heightMap);
    
    PerlinContent       mPerlinContent;
    ci::Vec3i           mGroundPlotCoords;
    std::map<float, ci::gl::TextureRef> mGroundMaps;
    ci::Vec3f           mGroundScale;
    
    std::map<int, GroundOrientaion> mTankGroundOrientations;
    float               mTankDirectionRadians;
    
};
}