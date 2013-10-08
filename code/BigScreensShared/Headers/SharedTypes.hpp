//
//  SharedTypes.hpp
//  GridMaker
//
//  Created by William Lindmeier on 10/8/13.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Rand.h"
#include "cinder/Rect.h"

namespace bigscreens
{    
    struct ScreenRegion
    {
        ci::Rectf rect;
        bool isActive;
        ci::ColorAf color;
        
        ScreenRegion(int x1, int y1, int x2, int y2) :
        rect(x1,y1,x2,y2),
        isActive(false),
        color(1.0f,1.0f,1.0f,1.0f)
        {
        };

        ScreenRegion(const ScreenRegion & reg) :
        rect(reg.rect),
        isActive(reg.isActive),
        color(reg.color)
        {
        };
    };
}