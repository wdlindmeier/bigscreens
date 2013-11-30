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
	
void Opponent::update(long numFramesRendered,
                      float percentage,
                      const ci::Vec3f & smokeAccel )
{
	// Smoke should have the opposite accel as movement

    float progress = numFramesRendered * 0.001f;
    
	if( mUpdateG )
    {
		mDynamicGeometry->update( percentage, progress);
		mUpdateG = false;
	}
    
	mSmokeEffect->update( smokeAccel, numFramesRendered );
}
	
void Opponent::draw(float zDepth,
                    const ci::Vec3f & lightPosition,
                    long numFramesRendered)
{
	// CAMERAVIEW - Will be used for lightPosition
	// zDepth - Used for particle smoke
	
    // TODO: This needs alpha for the final transition.
	ci::gl::enableDepthRead();
	ci::gl::enableDepthWrite();

	mDynamicGeometry->draw( lightPosition );

	ci::gl::disableDepthWrite();
	
	mSmokeEffect->draw( zDepth, numFramesRendered );
	
	ci::gl::disableDepthRead();
}
	
void Opponent::loadShaders()
{
	mDynamicGeometry->loadShaders();
	mSmokeEffect->loadShaders();
}
	
}