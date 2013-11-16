//
//  DumbTankContent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/14/13.
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
    class DumbTankContent : public TankContent
    {
        
    public:
        
        DumbTankContent();
        ~DumbTankContent(){};
        /*
        static PositionOrientation positionForTankWithProgress(const int tankNum,
                                                           long msOffset);
        static CameraOrigin cameraForTankConvergence(int regionIndex,
                                                     int regionCount,
                                                     long msOffset,
                                                     const ci::Vec2i & masterSize,
                                                     const ci::Rectf & regionRect);
        */
        // virtual void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
        // void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect, const float alpha);
        virtual void update(std::function<void (ci::CameraPersp & cam, DumbTankRef& tank)> update_func);
        /*
        void drawGround();
        void setMSElapsed(const long msElapsedConvergence);
        void drawScreen(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
        */
        virtual void loadShaders();
        
    protected:
        
        // virtual void drawTank();
        virtual void drawMinion();
        virtual void drawTankShots();
        virtual void renderPositionedTank();
        //void drawSingleTankAtPosition(const ci::Vec3f & position, const float rotationDegrees);
        /*
        float mRenderAlpha;
        float mScreenAlpha;
        long mMSElapsedConvergence;
        */
        DumbTankRef mDumbTank;
        ci::gl::GlslProgRef mTankShader;
        
    };
}