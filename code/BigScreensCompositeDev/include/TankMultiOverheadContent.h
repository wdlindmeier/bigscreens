//
//  TankMultiOverheadContent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/18/13.
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
#include "DumbTankContent.h"
#include "cinder/TriMesh.h"
#include "ContentProvider.h"

namespace bigscreens
{
    static const int kNumTanksStreaming = 20;
    
    class TankMultiOverheadContent : public DumbTankContent
    {
    public:
        
        TankMultiOverheadContent(){};
        ~TankMultiOverheadContent(){};
        
        static PositionOrientation positionForTankWithProgress(const int tankNum,
                                                               const long numFramesProgress);
        
    protected:
        
        void drawTank();
        void renderPositionedTank();
    };
}