//
//  Content.h
//  CinderProject
//
//  Created by Ryan Bartley on 10/4/13.
//
//

#pragma once

#include <map>
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Matrix44.h"

using namespace ci;

namespace bigscreens {

class Content {
public:
	Content( )
	{
		mCam.setPerspective(60, (float)640 / 480 , 1, 1000);
		mCam.lookAt( Vec3f( 0,0,1 ), Vec3f( 0,0,-1 ) );
	}
	~Content() {}
	
	CameraPersp& getCamera() { return mCam; }
	
	void render()
	{
		gl::bindStockShader( gl::ShaderDef().color() );
		
		gl::pushMatrices();
		gl::setMatrices(mCam);
		gl::multModelView( Matrix44f::createTranslation( Vec3f( 0,0,-1 ) ) );
		gl::multModelView( Matrix44f::createScale( 10 ) );
		
		gl::drawCube( Vec3f(0,0,0), Vec3f(1,1,1));
		
		gl::popMatrices();
	}
	
private:
	CameraPersp mCam;
};

}