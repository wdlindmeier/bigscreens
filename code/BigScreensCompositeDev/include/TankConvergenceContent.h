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

namespace bigscreens {
    
    class TankConvergenceContent : public TankContent
    {
        
    public:
        
        TankConvergenceContent() : TankContent() {};
        virtual ~TankConvergenceContent(){};
        
    protected:
        
        // virtual void loadShaders();
        // virtual void drawGround();
        
        virtual void drawTank();
        virtual void drawSingleTankAtPosition(const ci::Vec3f & position, const float radianRotation);

    };
}