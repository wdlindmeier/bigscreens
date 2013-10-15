//
//  Utilities.hpp
//  GridMaker
//
//  Created by William Lindmeier on 10/10/13.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Rect.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "SharedTypes.hpp"
#include "GridLayout.h"
#include <time.h>
#include <sys/timeb.h>

namespace bigscreens
{
    template<typename T>
    static bool rectCompare(ci::RectT<T> rectA, ci::RectT<T> rectB)
    {
        return  (int)rectA.x1 == (int)rectB.x1 &&
        (int)rectA.x2 == (int)rectB.x2 &&
        (int)rectA.y1 == (int)rectB.y1 &&
        (int)rectA.y2 == (int)rectB.y2;
    }
    
    static int getMilliCount()
    {
        timeb tb;
        ftime(&tb);
        int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
        return nCount;
    }
    
    static int getMilliSpan(int nTimeStart)
    {
        int nSpan = getMilliCount() - nTimeStart;
        if(nSpan < 0)
            nSpan += 0x100000 * 1000;
        return nSpan;
    }
    
    static ci::Rectf rectfFromRectInt(ci::RectT<int> rectA)
    {
        return ci::Rectf((float)rectA.x1, (float)rectA.y1, (float)rectA.x2, (float)rectA.y2);
    }
    
    static bool rectsOverlap(ci::Rectf rectA, ci::Rectf rectB)
    {
        return rectA.contains(rectB.getUpperLeft()) ||
        rectA.contains(rectB.getUpperRight()) ||
        rectA.contains(rectB.getLowerLeft()) ||
        rectA.contains(rectB.getLowerRight()) ||
        rectB.contains(rectA.getUpperLeft()) ||
        rectB.contains(rectA.getUpperRight()) ||
        rectB.contains(rectA.getLowerLeft()) ||
        rectB.contains(rectA.getLowerRight());
    };
    
    static bool rectIsValid(const ci::Rectf & rect,
                     const std::vector<ScreenRegion> & regions,
                     const bool ignoreInactive = true)
    {
        bool didOverlap = false;
        for (int i = 0; i < regions.size(); ++i)
        {
            ScreenRegion reg = regions[i];
            if ((reg.isActive || !ignoreInactive) && rectsOverlap(reg.rect, rect))
            {
                didOverlap = true;
                break;
            }
        }
        return !didOverlap;
    }
    
    static ci::Rectf rectFromTwoPos(const ci::Vec2f & posA, const ci::Vec2f & posB)
    {
        int x1 = std::min(posA.x, posB.x);
        int x2 = std::max(posA.x, posB.x);
        int y1 = std::min(posA.y, posB.y);
        int y2 = std::max(posA.y, posB.y);
        return ci::Rectf(x1, y1, x2, y2);
    }
    
    static ci::Vec2i mousePositionSnappedToSize(const ci::Vec2i & pos, const int snapSize)
    {
        return pos - ci::Vec2i(pos.x % snapSize, pos.y % snapSize);
    }

    static bool sortByTimestamp(const GridLayout & layout1, const GridLayout & layout2)
    {
        return (layout1.getTimestamp() < layout2.getTimestamp());
    }
    
    static ci::fs::path SharedGridPath()
    {
        return cinder::app::getAssetPath(".") / ".." / ".." / "BigScreensShared" / "Assets" / "grid";
    }
	
	// This is for the shader path but I couldn't get it working like the above
	// TODO: get this working like the above
	static std::string SharedShaderPath()
	{
		return "../../BigScreensShared/Assets/shaders/";
	}
}