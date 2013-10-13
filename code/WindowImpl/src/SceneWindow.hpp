//
//  Window.h
//  CinderProject
//
//  Created by Ryan Bartley on 10/4/13.
//
//

#pragma once

#include "cinder/gl/Fbo.h"
#include "Content.h"

namespace bigscreens {

typedef std::shared_ptr<class SceneWindow> SceneWindowRef;
// Making this class more interested in the dimensions from
// an origin for opengl.
typedef std::pair<ci::Vec2i, ci::Vec2i> OriginAndDimension;
	
class SceneWindow {
public:
	SceneWindow( Content * subContent, ci::Rectf *origAndDim, ci::gl::FboRef scratch )
	: mContent( subContent ), mScratch( scratch ), mLastAspect( mContent->getCamera().getAspectRatio() ),
		mOrigAndDim(  Vec2i( origAndDim->x1, ci::app::getWindowHeight() - origAndDim->y2 ),
					  Vec2i( origAndDim->getWidth(), origAndDim->getHeight() ) )
	{
		// After we have the camera's original aspect ratio,
		// set the new aspect ratio to the dimensions of the window
		mContent->getCamera().setAspectRatio( (float)mOrigAndDim.second.x / mOrigAndDim.second.y );
		// Render the scene
		render();
	}
	
	~SceneWindow()
	{
		// On the way, reset the lastAspect ratio
		mContent->getCamera().setAspectRatio( mLastAspect );
	}
	
	void render()
	{
		// Only render exactly what we have to
		setScissorAndViewport();
		
		// Enable the test here
		gl::enable( GL_SCISSOR_TEST );
		mScratch->bindFramebuffer();
		
		gl::clear();
		
		// Render our scene to the fbo
		mContent->render();
		
		mScratch->unbindFramebuffer();
		gl::disable( GL_SCISSOR_TEST );
		// Disable the test
		
		// Blit the result to the screen,
		// May get rid of this if we're only
		// going to use one fbo
		blitToScreen();
	}
	
private:
	void setScissorAndViewport()
	{
		// This is LowerLeft and Width and Height
		gl::viewport( mOrigAndDim.first.x, mOrigAndDim.first.y, mOrigAndDim.second.x, mOrigAndDim.second.y );
		gl::scissor( mOrigAndDim.first.x, mOrigAndDim.first.y, mOrigAndDim.second.x, mOrigAndDim.second.y );
	}
	
	void blitToScreen()
	{
		mScratch->blitToScreen( ci::Area( mOrigAndDim.first.x, mOrigAndDim.first.y,
										  mOrigAndDim.first.x + mOrigAndDim.second.x, mOrigAndDim.first.y + mOrigAndDim.second.y ),
							   ci::Area( mOrigAndDim.first.x, mOrigAndDim.first.y,
										mOrigAndDim.first.x + mOrigAndDim.second.x, mOrigAndDim.first.y + mOrigAndDim.second.y ) );
	}
	
	
private:
	Content * mContent;
	OriginAndDimension mOrigAndDim;
	ci::gl::FboRef	mScratch;
	float mLastAspect;
};

}