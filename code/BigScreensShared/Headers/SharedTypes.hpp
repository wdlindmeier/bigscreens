//
//  SharedTypes.hpp
//  GridMaker
//
//  Created by William Lindmeier on 10/8/13.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "cinder/Rect.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"

namespace bigscreens
{
    class RenderableContent
    {
    public:
        virtual ~RenderableContent(){};
        virtual void render(const ci::Vec2i & screenOffset) = 0;
        // This is to manipulate the aspect ratio
		// so that we can use different windows
		// for our content.
		virtual ci::CameraPersp& getCamera() = 0;
    };

    typedef std::shared_ptr<class RenderableContent> RenderableContentRef;
    
    // We'll use a content proivder rather than a global map so we
    // can load and unload the content as needed, to save memory.
    class ContentProvider
    {

    public:
        
        virtual ~ContentProvider(){}
        // Passes back a pointer to some renderable content.
        virtual RenderableContentRef contentForKey(const std::string & contentName) = 0;
    };
    
    // SceneWindow type, more interested in the dimensions from
	// an origin for opengl.
	typedef std::pair<ci::Vec2i, ci::Vec2i> OriginAndDimension;

    struct ScreenRegion
    {
        ci::Rectf rect;
        bool isActive;
        bool isSelected;
        ci::ColorAf color;
        std::string contentKey;

        ScreenRegion(int x1, int y1, int x2, int y2) :
        rect(x1,y1,x2,y2),
        isActive(false),
        isSelected(false)
        {
        };

		// Helper function to get the translated Origin and Dimension
		// from the region to the SceneWindow
		OriginAndDimension getOriginAndDimension()
		{
			return OriginAndDimension( ci::Vec2i( rect.x1, ci::app::getWindowHeight() - rect.y2 ),
									   ci::Vec2i( rect.getWidth(), rect.getHeight() ) );
		}
    };
    
}