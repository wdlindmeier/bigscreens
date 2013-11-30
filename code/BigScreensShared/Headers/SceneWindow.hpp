//
//  Window.h
//  CinderProject
//
//  Created by Ryan Bartley on 10/4/13.
//
//

#pragma once

#include "cinder/gl/Fbo.h"
#include "cinder/gl/gl.h"
#include "SharedTypes.hpp"

namespace bigscreens
{
    
    typedef std::shared_ptr<class SceneWindow> SceneWindowRef;
	
    class SceneWindow
    {
        
    public:
        
        SceneWindow(RenderableContentRef content,
                    const ci::Rectf & screenRegion,
                    const ci::Vec2i & screenSize) :
        mContentRect( screenRegion ),
        mContent( content ),
        mLastAspect( mContent->getCamera().getAspectRatio() ),
		mOrigAndDim( OriginAndDimensionFromRectf(screenRegion, screenSize.y) )
        {
        }
        
        ~SceneWindow()
        {
        }
        
        // Offset is the MPE window offset
        void render(const ci::Vec2i & offset)
        {
            mContent->getCamera().setAspectRatio( (float)mOrigAndDim.second.x / mOrigAndDim.second.y );

            setScissorAndViewport(offset);
            
            ci::gl::enable( GL_SCISSOR_TEST );

            mContent->render(offset, mContentRect);

            ci::gl::disable( GL_SCISSOR_TEST );
            
        }

    private:
        
        void setScissorAndViewport(const ci::Vec2i & offset)
        {
            // This is LowerLeft and Width and Height
            ci::gl::viewport(mOrigAndDim.first.x - offset.x,
                             mOrigAndDim.first.y - offset.y,
                             mOrigAndDim.second.x,
                             mOrigAndDim.second.y );
            ci::gl::scissor(mOrigAndDim.first.x - offset.x,
                            mOrigAndDim.first.y - offset.y,
                            mOrigAndDim.second.x,
                            mOrigAndDim.second.y );
        }
        
    private:
        
        ci::Rectf               mContentRect;
        RenderableContentRef    mContent;
        OriginAndDimension      mOrigAndDim;
        float                   mLastAspect;
    };
    
}