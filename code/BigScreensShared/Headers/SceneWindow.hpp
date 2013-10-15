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
#include "OutLineBorder.hpp"

namespace bigscreens {

typedef std::shared_ptr<class SceneWindow> SceneWindowRef;
	
class SceneWindow {
public:
	SceneWindow( RenderableContent * subContent, ScreenRegion *origAndDim, ci::gl::FboRef scratch, OutLineBorderRef outLine )
	: mContent( subContent ), mScratch( scratch ), mLastAspect( mContent->getCamera().getAspectRatio() ),
		mOrigAndDim( origAndDim->getOriginAndDimension() ), mOutLine( outLine )
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
		ci::gl::enable( GL_SCISSOR_TEST );
		mScratch->bindFramebuffer();
		
		ci::gl::clear( ci::ColorA( 1.0f, 0.0f, 0.0f, 1.0f ) );
		
		// Render our scene to the fbo
		mContent->render();
		
		renderOutline();
		
		mScratch->unbindFramebuffer();
		ci::gl::disable( GL_SCISSOR_TEST );
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
		ci::gl::viewport( mOrigAndDim.first.x, mOrigAndDim.first.y, mOrigAndDim.second.x, mOrigAndDim.second.y );
		ci::gl::scissor( mOrigAndDim.first.x, mOrigAndDim.first.y, mOrigAndDim.second.x, mOrigAndDim.second.y );
	}
	
	void blitToScreen()
	{
		mScratch->blitToScreen( ci::Area( mOrigAndDim.first.x, mOrigAndDim.first.y,
										  mOrigAndDim.first.x + mOrigAndDim.second.x, mOrigAndDim.first.y + mOrigAndDim.second.y ),
							   ci::Area( mOrigAndDim.first.x, mOrigAndDim.first.y,
										mOrigAndDim.first.x + mOrigAndDim.second.x, mOrigAndDim.first.y + mOrigAndDim.second.y ) );
	}
	
	void renderOutline()
	{
		mOutLine->render();
	}
	
	
private:
	RenderableContent * mContent;
	OriginAndDimension	mOrigAndDim;
	ci::gl::FboRef		mScratch;
	float				mLastAspect;
	OutLineBorderRef	mOutLine;
};

}