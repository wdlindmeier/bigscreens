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
	
void Opponent::update( float percentage, const ci::Vec3f & smokeAccel )
{
	// Smoke should have the opposite accel as movement
	float deltaT = (ci::app::getElapsedSeconds() / 1000) - mTime;
	mTime = ci::app::getElapsedSeconds() / 1000;
	if( mUpdateG ) {
		mDynamicGeometry->update( percentage, mTime );
		mUpdateG = false;
	}
//	mSmokeEffect->update( smokeAccel, mTime );
}
	
void Opponent::draw( float zDepth, const ci::Vec3f & cameraView )
{
	// CAMERAVIEW - Will be used for lightPosition
	// zDepth - Used for particle smoke
	
	ci::gl::enableDepthRead();
	ci::gl::enableDepthWrite();

	mDynamicGeometry->draw( cameraView );

	ci::gl::disableDepthWrite();
	
//	mSmokeEffect->draw( zDepth );
	
	ci::gl::disableDepthRead();
}
	
void Opponent::loadShaders()
{
	mDynamicGeometry->loadShaders();
	mSmokeEffect->loadShaders();
}
	
}