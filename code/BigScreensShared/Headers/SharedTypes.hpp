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
    
    typedef std::pair<std::string, int> TextWithFramecount;
    typedef std::vector<TextWithFramecount> TextTimeline;

    class RenderableContent
    {
    public:
        RenderableContent() : mContentID(-1), mNumFramesRendered(0){};
        virtual ~RenderableContent(){};
        virtual void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect) = 0;
        virtual void setFramesRendered(const long long numFramesRendered){ mNumFramesRendered = numFramesRendered; };
        long long getFramesRendered(){ return mNumFramesRendered; };
        // NOTE: Set a tmp content ID before rendering
        // and before firing a shot. All shots are tied to
        // a specific content ID.
        // Content ID is cleared after the tank has been rendered.
        virtual void setFrameContentID(const int contentID){ mContentID = contentID; };
        // This is to manipulate the aspect ratio
		// so that we can use different windows
		// for our content.
		virtual ci::Camera & getCamera() = 0;
        virtual bool drawsOutline(){ return true; }
        
    protected:
        long long mNumFramesRendered;
        int mContentID;
    };

    typedef std::shared_ptr<class RenderableContent> RenderableContentRef;
    
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
        int timelineID;

        ScreenRegion(int x1, int y1, int x2, int y2) :
        rect(x1,y1,x2,y2),
        isActive(false),
        isSelected(false),
        color(ci::ColorAf(1.0f,1.0f,1.0f,1.0f)),
        timelineID(0)
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
    
    // This is a data model for every region that's currently on screen.
    // NOTE: Not all will actually need to be rendered or updated if they
    // aren't visible on the current screen.
    struct TimelineContentInfo
    {
        long long numRenderFrames = 0;
        std::string contentKey = "";
        ci::Rectf rect = ci::Rectf(0,0,0,0);
        RenderableContentRef contentRef;
        int layoutIndex;
    };
    
    struct CameraOrigin
    {
        ci::Vec3f eye;
        ci::Vec3f target;
        ci::Vec2f camShift;
    };
    
    struct GroundOrientaion
    {
        float height = 0;
        float xAngleRads = 0;
        float zAngleRads = 0;
    };
    
    struct PositionOrientation
    {
        ci::Vec3f position = ci::Vec3f::zero();
        ci::Vec3f vector = ci::Vec3f::zero();
        float directionDegrees = 0;
    };
    
}