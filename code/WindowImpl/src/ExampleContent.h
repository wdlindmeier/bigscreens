//
//  ExampleContent.h
//  WindowImpl
//
//  Created by Ryan Bartley on 10/8/13.
//
//

#pragma once

#include "Content.h"

namespace bigscreens {

class ExampleContent : public Content  {
public:
	
	ExampleContent() : Content()
	{
		mCam.setPerspective(60, (float)640 / 480 , 1, 1000);
		mCam.lookAt( Vec3f( 0,0,1 ), Vec3f( 0,0,-1 ) );
		rotation = 0;
	}
	
	~ExampleContent() {}
	
	void render()
	{
		gl::bindStockShader( gl::ShaderDef().color() );
		
		rotation++;
		gl::pushMatrices();
		gl::setMatrices(mCam);
		gl::multModelView( Matrix44f::createTranslation( Vec3f( 0,0,-1 ) ) );
		gl::multModelView( Matrix44f::createRotation( Vec3f( 1, 0, 0 ), toRadians( rotation ) ) );
		gl::multModelView( Matrix44f::createScale( 10 ) );
		
		gl::drawCube( Vec3f(0,0,0), Vec3f(1,1,1));
		
		gl::popMatrices();
	}
	
	CameraPersp& getCamera() { return mCam; }
	
private:
	CameraPersp mCam;
	float rotation;
};
	
}
