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
	SceneWindow( Content * subContent, OriginAndDimension * origAndDim, ci::gl::FboRef scratch, ColorA clearColor = ColorA::black(), float clearDepth = 1.0f ) : content( subContent ), origAndDim(origAndDim), mScratch( scratch ), clearColor(clearColor), clearDepth(clearDepth) { render(); }
	~SceneWindow() {}
	
	void render()
	{
		mScratch->bindFramebuffer();
		setScissorAndViewport();
		
		gl::clearColor( clearColor );
		gl::clear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT ) ;
		
		content->render();
		
		
		mScratch->unbindFramebuffer();
		blitToScreen();
	}
	
private:
	void setScissorAndViewport()
	{
		gl::viewport( origAndDim->first, origAndDim->second );
		gl::scissor( origAndDim->first, origAndDim->second );
	}
	
	void blitToScreen()
	{
		mScratch->blitToScreen( ci::Area( origAndDim->first.x, origAndDim->first.y, origAndDim->second.x, origAndDim->second.y ), ci::Area( origAndDim->first.x, origAndDim->first.y, origAndDim->second.x, origAndDim->second.y ) );
	}
	
	
private:
	Content * content;
	OriginAndDimension * origAndDim;
	ci::gl::FboRef	mScratch;
	ColorA clearColor;
	float clearDepth;
};

}