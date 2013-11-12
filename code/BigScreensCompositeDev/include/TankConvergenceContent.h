//
//  TankConvergenceContent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/31/13.
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
#include "TankContent.h"
#include "cinder/TriMesh.h"
#include "ContentProvider.h"

namespace bigscreens
{
   static const int kNumTanksConverging = 20;
    
    struct TankOrientation
    {
        ci::Vec3f position;
        float directionDegrees;
    };
    
    class TankConvergenceContent : public TankContent
    {
        
    public:
        
        TankConvergenceContent();
        ~TankConvergenceContent(){};
        static TankOrientation positionForTankWithProgress(const int tankNum,
                                                           long msOffset);
        static CameraOrigin cameraForTankConvergence(int regionIndex,
                                                     int regionCount,
                                                     long msOffset,
                                                     const ci::Vec2i & masterSize,
                                                     const ci::Rectf & regionRect);
        
        void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
        void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect, const float alpha);
        void update(std::function<void (ci::CameraPersp & cam, DumbTankRef& tank)> update_func);
        void drawGround();
        void setMSElapsed(const long msElapsedConvergence);
        void drawScreen(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
        virtual void loadShaders();

    protected:

        void drawTank();
        void drawSingleTankAtPosition(const ci::Vec3f & position, const float rotationDegrees);
        
        float mRenderAlpha;
        float mScreenAlpha;
        long mMSElapsedConvergence;
        
        DumbTankRef mDumbTank;
    };
}