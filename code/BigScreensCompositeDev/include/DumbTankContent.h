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
        virtual void update(std::function<void (ci::CameraPersp & cam, DumbTankRef& tank)> update_func);
        virtual void loadShaders();
        virtual void fireTankGun();
        virtual void setFrameContentID(const int contentID);
        
    protected:

        virtual void drawMinion();
        virtual void drawTankShots();
        virtual void renderPositionedTank();
        DumbTankRef mDumbTank;
        ci::gl::GlslProgRef mTankShader;
        
    };
}