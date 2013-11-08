//
//  Opponent.cpp
//  Opponent
//
//  Created by Ryan Bartley on 11/4/13.
//
//

#include "Opponent.h"
#include "Utilities.hpp"

namespace bigscreens {
	
void Opponent::draw()
{
	ci::gl::enableDepthRead();
	ci::gl::enableDepthWrite();
	ci::gl::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	mGlsl->bind();
	
	mGlsl->uniform( "projection", ci::gl::getProjection() );
	mGlsl->uniform( "modelView", ci::gl::getModelView() );
	
	
	glPointSize(10.0f);
	
	mSphericalGeometry->draw();
	mPyramidalGeometry->draw();
	
	mGlsl->unbind();
	
	ci::gl::disableDepthRead();
	ci::gl::disableDepthWrite();
	
	mSmokeEffect->draw();
	
	ci::gl::popMatrices();
	
}
	
void Opponent::loadShaders()
{
	ci::gl::GlslProg::Format mFormat;
	mFormat.vertex( ci::app::loadAsset( /*"oppBasic.vert"*/ SharedShaderAssetPath("oppBasic.vert", !IS_IAC) ) )
	.fragment( ci::app::loadAsset( /*"oppBasic.frag"*/ SharedShaderAssetPath("oppBasic.frag", !IS_IAC) ) );
	mGlsl = ci::gl::GlslProg::create( mFormat );
}
	
}