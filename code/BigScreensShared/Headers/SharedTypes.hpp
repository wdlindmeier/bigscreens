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

namespace bigscreens
{
	// SceneWindow type, more interested in the dimensions from
	// an origin for opengl.
	typedef std::pair<ci::Vec2i, ci::Vec2i> OriginAndDimension;
	
    class RenderableContent {
	public:
		RenderableContent() {}
		virtual ~RenderableContent() {}
		
		// This is to manipulate the aspect ratio
		// so that we can use different windows
		// for our content
		virtual ci::CameraPersp& getCamera() = 0;
		
		// This is what every renderable class
		// should have
		virtual void render() = 0;
		
	private:
		
	};
    
    struct ScreenRegion
    {
        ci::Rectf rect;
        bool isActive;
        bool isSelected;
        
        ScreenRegion(int x1, int y1, int x2, int y2) :
        rect(x1,y1,x2,y2),
        isActive(false),
        isSelected(false)
        {
        };

        // Is this necessary?
        ScreenRegion(const ScreenRegion & reg) :
        rect(reg.rect),
        isActive(reg.isActive),
        isSelected(reg.isSelected)
        {
        };
		
		OriginAndDimension getOriginAndDimension()
		{
			return OriginAndDimension( ci::Vec2i( rect.x1, ci::app::getWindowHeight() - rect.y2 ),
									   ci::Vec2i( rect.getWidth(), rect.getHeight() ) );
		}
		
		float * getOutline()
		{
			return new float[8] { rect.x1, rect.y1,
								  rect.x1, rect.y1 + rect.y2,
								  rect.x1 + rect.x2, rect.y1 + rect.y2,
								  rect.x1 + rect.x2, rect.y1 };
		}
    };
    
    struct Slider
    {
        Slider(ci::Rectf rect) : mRect(rect), mValue(0){};
        Slider(){};
        ~Slider(){};
        
        ci::Rectf mRect;
        float mValue;
        bool mIsActive;
        
        void update(ci::Vec2f mousePosition)
        {
            ci::Vec2f relPos = mousePosition - mRect.getUpperLeft();
            float scalarVal = relPos.x / mRect.getWidth();
            mValue = std::min<float>(1.0f, std::max<float>(0.0f, scalarVal));
        }
        
        bool contains(ci::Vec2f position)
        {
            return mRect.contains(position);
        }
        
        float getValue()
        {
            return mValue;
        }
        
        void setValue(const float value)
        {
            mValue = value;
        }
        
        void setIsActive(bool isActive)
        {
            mIsActive = isActive;
        }
        
        bool getIsActive()
        {
            return mIsActive;
        }
        
        void render(bool isEnabled)
        {
//            ci::gl::lineWidth(1.0f);
//            ci::gl::color(ci::Color::white());
//            ci::gl::drawLine(ci::Vec2f(mRect.x1, mRect.getCenter().y),
//                         ci::Vec2f(mRect.x2, mRect.getCenter().y));
//            ci::gl::lineWidth(6.0f);
//            if (isEnabled)
//            {
//                ci::gl::color(ci::ColorAf(1.0f, 0, 0, 1.0f));
//            }
//            else
//            {
//                ci::gl::color(ci::ColorAf(0.5f, 0.5f, 0.5f, 1.0f));
//            }
//            float offsetX = mRect.getWidth() * mValue;
//            ci::gl::drawLine(ci::Vec2f(mRect.x1 + offsetX,
//                                       mRect.y1),
//                             ci::Vec2f(mRect.x1 + offsetX,
//                                       mRect.y2));
        }
    };
    

}