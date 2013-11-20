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
#include "SmokeEffect.h"

namespace bigscreens {
	
typedef std::shared_ptr<class Opponent> OpponentRef;
	
class Opponent {
public:
	Opponent()
	: mDynamicGeometry( new DynamicOpponent() ),
		mSmokeEffect( new SmokeEffect() ),
		mTime( ci::app::getElapsedSeconds() ),
		mUpdateG( false )
	{
	}
	
	void update( float percentage, const ci::Vec3f & smokeAccel );
	void draw( float zDepth, const ci::Vec3f & lightPosition );
	
	void loadShaders();
	
	void setUpdateGeometry( bool update ) { mUpdateG = update; }
	
private:
	
private:
	DynamicOpponentRef		mDynamicGeometry;
	SmokeEffectRef			mSmokeEffect;
	float					mTime;
	bool					mUpdateG;
};
	
}
