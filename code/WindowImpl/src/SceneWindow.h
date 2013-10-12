//
//  Window.h
//  CinderProject
//
//  Created by Ryan Bartley on 10/4/13.
//
//

#pragma once

#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "Content.h"

namespace bigscreens {

typedef std::shared_ptr<class SceneWindow> SceneWindowRef;
typedef std::pair<ci::Vec2i, ci::Vec2i> OriginAndDimension;
	
class SceneWindow {
public:
	SceneWindow( Content * subContent, ci::Rectf *origAndDim, ci::gl::FboRef scratch, ci::gl::FboRef accumulation, ColorA color )
	: mContent( subContent ), mScratch( scratch ), mAccumulation( accumulation ), mClearColor( color ), 
		mOrigAndDim( new OriginAndDimension( Vec2i( origAndDim->x1, ci::app::getWindowHeight() - origAndDim->y2 ), Vec2i( origAndDim->getWidth(), origAndDim->getHeight() ) ) ),
		mLastAspect( mContent->getCamera().getAspectRatio() )
	{
		mContent->getCamera().setAspectRatio( (float)mOrigAndDim->second.x / mOrigAndDim->second.y );
		render();
	}
	~SceneWindow()
	{
		
		mContent->getCamera().setAspectRatio( mLastAspect );
		delete mOrigAndDim;
	}
	
	void render()
	{
		setScissorAndViewport();
		
		gl::enable( GL_SCISSOR_TEST );
		mScratch->bindFramebuffer();
		
		gl::clear( mClearColor );
		
		mContent->render();
		
		mScratch->unbindFramebuffer();
		gl::disable( GL_SCISSOR_TEST );
		
		blitToScreen();
	}
	
private:
	void setScissorAndViewport()
	{
		gl::viewport( mOrigAndDim->first.x, mOrigAndDim->first.y, mOrigAndDim->second.x, mOrigAndDim->second.y );
		gl::scissor( mOrigAndDim->first.x, mOrigAndDim->first.y, mOrigAndDim->second.x, mOrigAndDim->second.y );
	}
	
	void blitToScreen()
	{
//		mScratch->blitTo( mAccumulation, *mOrigAndDim, *mOrigAndDim );
		mScratch->blitToScreen( ci::Area( mOrigAndDim->first.x, mOrigAndDim->first.y,
										  mOrigAndDim->first.x + mOrigAndDim->second.x, mOrigAndDim->first.y + mOrigAndDim->second.y ),
							   ci::Area( mOrigAndDim->first.x, mOrigAndDim->first.y,
										mOrigAndDim->first.x + mOrigAndDim->second.x, mOrigAndDim->first.y + mOrigAndDim->second.y ) );
	}
	
	
private:
	Content * mContent;
	OriginAndDimension * mOrigAndDim;
	ci::gl::FboRef	mScratch, mAccumulation;
	ColorA mClearColor;
	float mLastAspect;
};

}