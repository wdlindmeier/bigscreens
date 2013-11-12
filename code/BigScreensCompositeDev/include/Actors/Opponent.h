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
//		mMinionGeometry = MinionGeometryRef( new MinionGeometry() );
//		mSphericalGeometry = SphericalGeometryRef( new SphericalGeometry(false) );
		mSmokeEffect = SmokeEffectRef( new SmokeEffect() );
		mDynamicGeometry = DynamicOpponentRef( new DynamicOpponent() );
		
		loadShaders();
	}
	
	void update( float percentage );
	void draw( float zDepth, const ci::Vec3f & cameraView );
	
	void loadShaders();
	void loadTextures();
	
private:
	
private:
	MinionGeometryRef		mMinionGeometry;
	DynamicOpponentRef		mDynamicGeometry;
	SphericalGeometryRef	mSphericalGeometry;
	SmokeEffectRef			mSmokeEffect;
	ci::gl::GlslProgRef		mGlsl;
	GLuint					renderSub, updateSub;
};
	
}
