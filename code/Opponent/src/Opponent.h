//
//  Opponent.h
//  Opponent
//
//  Created by Ryan Bartley on 11/4/13.
//
//

#pragma once

#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "SmokeEffect.hpp"
#include "OppDynamicGeom.hpp"

namespace bigscreens {
	
typedef std::shared_ptr<class Opponent> OpponentRef;
	
class Opponent {
public:
	Opponent()
	{
		mPyramidalGeometry = PyramidalGeometryRef( new PyramidalGeometry() );
		mSphericalGeometry = SphericalGeometryRef( new SphericalGeometry() );
		mSmokeEffect = SmokeEffectRef( new SmokeEffect() );
		
		loadShaders();
	}
	
	void draw()
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
	
private:
	
	void loadShaders()
	{
		ci::gl::GlslProg::Format mFormat;
		mFormat.vertex( ci::app::loadResource("oppBasic.vert") ).fragment( ci::app::loadResource("oppBasic.frag" ) );
		mGlsl = ci::gl::GlslProg::create( mFormat );
	}
	
private:
	PyramidalGeometryRef	mPyramidalGeometry;
	SphericalGeometryRef	mSphericalGeometry;
	SmokeEffectRef			mSmokeEffect;
	ci::gl::GlslProgRef		mGlsl;
};
	
}
