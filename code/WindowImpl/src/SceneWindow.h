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
	SceneWindow( Content * subContent, ci::Area *origAndDim, ci::gl::FboRef scratch, ci::gl::FboRef accumulation, ColorA color )
	: mContent( subContent ), mScratch( scratch ), mAccumulation( accumulation ), mClearColor( color ), 
		mOrigAndDim( new ci::Area( Vec2i( origAndDim->x1, ci::app::getWindowHeight() - origAndDim->y2 ), Vec2i( origAndDim->x2, origAndDim->y1 ) ) ),
		mLastAspect( mContent->getCamera().getAspectRatio() ), mWidth( origAndDim->getWidth() ), mHeight( origAndDim->getHeight() )
	{
		mContent->getCamera().setAspectRatio( (float)mWidth / mHeight );
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
		mScratch->bindFramebuffer();
		
		gl::clear( mClearColor );
		
		mContent->render();
		
		mScratch->unbindFramebuffer();
		blitToScreen();
	}
	
private:
	void setScissorAndViewport()
	{
		gl::viewport( mOrigAndDim->x1, mOrigAndDim->y1, mWidth, mHeight );
		gl::scissor( mOrigAndDim->x1, mOrigAndDim->y2, mWidth, mHeight );
	}
	
	void blitToScreen()
	{
		mScratch->blitTo( mAccumulation, *mOrigAndDim, *mOrigAndDim ); 
	}
	
	
private:
	Content * mContent;
	Area * mOrigAndDim;
	ci::gl::FboRef	mScratch, mAccumulation;
	ColorA mClearColor;
	float mLastAspect;
	int mWidth, mHeight;
};

}