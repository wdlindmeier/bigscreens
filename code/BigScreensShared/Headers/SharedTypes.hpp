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
#include "cinder/gl/Texture.h"

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

template<typename T>
static bool RectCompare(ci::RectT<T> rectA, ci::RectT<T> rectB)
{
    return  (int)rectA.x1 == (int)rectB.x1 &&
            (int)rectA.x2 == (int)rectB.x2 &&
            (int)rectA.y1 == (int)rectB.y1 &&
            (int)rectA.y2 == (int)rectB.y2;
}