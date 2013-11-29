//
//  FinalBillboard.h
//  BigScreensComposite
//
//  Created by Ryan Bartley on 11/1/13.
//
//

#pragma once

#include "cinder/gl/Context.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "Utilities.hpp"

namespace bigscreens {
	
typedef std::shared_ptr<class FinalBillboard> FinalBillboardRef;
	
class FinalBillboard
{
    
public:
    
	FinalBillboard();
	~FinalBillboard() {}
	
	void setupBuffers();
	void loadShaders();
	void draw( const ci::gl::TextureRef billboardTex, long numFramesRendered );
    
private:
    
	ci::gl::VaoRef		mBillboardVao;
	ci::gl::VboRef		mBillboardVbo, mBillboardElementVbo;
	ci::gl::GlslProgRef	mEffectsGlsl;
};
	
}
