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
        //ci::gl::FboRef fbo) :
        mContent( content ),
        mLastAspect( mContent->getCamera().getAspectRatio() ),
		mOrigAndDim( OriginAndDimensionFromRectf(screenRegion, screenSize.y) )
        //mOrigAndDim( OriginAndDimensionFromRectf(screenRegion, fbo->getHeight()) ),
        //mFBO(fbo)
        {
        }
        
        ~SceneWindow()
        {
        }
        
        // Offset is the MPE window offset
        void render(const ci::Vec2i & offset)
        {
            // Render the scene
            
            // After we have the camera's original aspect ratio,
            // set the new aspect ratio to the dimensions of the window
            
            // NOTE: Moved this from the constructor.
            // It feels more semantically appropriate here, and in practice
            // it's called at the same time, but leaves us the option of caching a scene window.
            
            mLastAspect = mContent->getCamera().getAspectRatio();
            mContent->getCamera().setAspectRatio( (float)mOrigAndDim.second.x / mOrigAndDim.second.y );
            // Only render exactly what we have to
            
            setScissorAndViewport(offset);
            
            ci::gl::enable( GL_SCISSOR_TEST );
            //mFBO->bindFramebuffer();
            
            mContent->render(offset);

            //mFBO->unbindFramebuffer();
            ci::gl::disable( GL_SCISSOR_TEST );
            
            // renderOutline();
            
            
            //blitToScreen();
            
            
            // Reset the aspect ratio.
            // NOTE: Moved this from the destructor.
            mContent->getCamera().setAspectRatio( mLastAspect );
            
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
        
        // We may want to remove the FBO
        // This currently just adds overhead.
        /*
         void blitToScreen()
         {
            mFBO->blitToScreen(ci::Area(mOrigAndDim.first.x, mOrigAndDim.first.y,
                                        mOrigAndDim.first.x + mOrigAndDim.second.x,
                                        mOrigAndDim.first.y + mOrigAndDim.second.y ),
                               ci::Area(mOrigAndDim.first.x, mOrigAndDim.first.y,
                                        mOrigAndDim.first.x + mOrigAndDim.second.x,
                                        mOrigAndDim.first.y + mOrigAndDim.second.y ) );
        }
        */

    private:
        
        RenderableContentRef    mContent;
        OriginAndDimension      mOrigAndDim;
        float                   mLastAspect;
        //ci::gl::FboRef          mFBO;
    };
    
}