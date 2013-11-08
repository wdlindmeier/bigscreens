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
#include "OpponentGeometry.h"
#include "SmokeEffect.hpp"

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
	
	void draw();
	
private:
	
	void loadShaders();
	
private:
	PyramidalGeometryRef	mPyramidalGeometry;
	SphericalGeometryRef	mSphericalGeometry;
	SmokeEffectRef			mSmokeEffect;
	ci::gl::GlslProgRef		mGlsl;
};
	
}
