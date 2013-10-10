//
//  SharedTypes.hpp
//  GridMaker
//
//  Created by William Lindmeier on 10/8/13.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Rect.h"

namespace bigscreens
{
    class RenderableContent
    {
    public:
        virtual void render() = 0;
        virtual ci::gl::Texture getTexture() = 0;
    };
    
    struct ScreenRegion
    {
        ci::Rectf rect;
        bool isActive;
        bool isSelected;
        ci::ColorAf color;
        
        ScreenRegion(int x1, int y1, int x2, int y2) :
        rect(x1,y1,x2,y2),
        isActive(false),
        isSelected(false),
        color(1.0f,1.0f,1.0f,1.0f)
        {
        };

        // Is this necessary?
        ScreenRegion(const ScreenRegion & reg) :
        rect(reg.rect),
        isActive(reg.isActive),
        isSelected(reg.isSelected),
        color(reg.color)
        {
        };
    };
}