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
    
    const static long kNumFramesTanksConverge = 1000;
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
        virtual ~TankConvergenceContent(){};
        TankOrientation positionForTankWithProgress(const int tankNum,
                                                    long frameProgress);
        
        void render(const ci::Vec2i & screenOffset);
        void render(const ci::Vec2i & screenOffset, const float alpha);

    protected:

        virtual void drawTank();
        virtual void drawSingleTankAtPosition(const ci::Vec3f & position, const float rotationDegrees);
        
        float mRenderAlpha;

    };
}