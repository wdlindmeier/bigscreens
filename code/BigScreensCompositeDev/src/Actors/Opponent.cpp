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
	
void Opponent::update( float percentage )
{
	mDynamicGeometry->update( percentage );
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
}
	
void Opponent::loadTextures()
{
	
}
	
}