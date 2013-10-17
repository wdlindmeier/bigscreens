//
//  TankContent.cpp
//  GridMaker
//
//  Created by William Lindmeier on 10/8/13.
//
//

#include "TankContent.h"
#include "cinder/ObjLoader.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include "cinder/Camera.h"

using namespace ci;
using namespace ci::app;

static const int FBO_WIDTH = 512, FBO_HEIGHT = 512;

void TankContent::load(const std::string & objFilename)
{
    DataSourceRef file = loadResource( objFilename ); //"T72.obj"
    ObjLoader loader( file );
	loader.load( &mMesh );
	mVBO = gl::VboMesh( mMesh );
    mRotation.setToIdentity();
    gl::Fbo::Format format;
	format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
	format.enableColorBuffer( true, 2 ); // create an FBO with two color attachments
	mFBO = gl::Fbo( FBO_WIDTH, FBO_HEIGHT, format );
}

void TankContent::reset()
{
    mRotation.setToIdentity();
}

void TankContent::update()
{
    mRotation.rotate( Vec3f( 0, 1, 0 ), 0.006f );
}

void TankContent::render()
{
    // bind the framebuffer - now everything we draw will go there
	mFBO.bindFramebuffer();
    
    gl::enableAlphaBlending();
    gl::enableAdditiveBlending();
    
	// setup the viewport to match the dimensions of the FBO
	gl::setViewport( mFBO.getBounds() );

	// clear out both of the attachments of the FBO with black
	gl::clear( ColorAf( 0.0f, 0.0f, 0.0f, 0.0f ) );
    
    CameraPersp cam(mFBO.getWidth(),
                    mFBO.getHeight(),
                    45.0f );
	gl::setMatrices( cam );
    
    gl::pushMatrices();
    gl::translate(mFBO.getWidth() * 0.5f,
                  mFBO.getHeight() * 0.7f,
                  -600.0f);
    // Flip vertically
    gl::rotate(Vec3f(180.0f, 0, 0));
    gl::multModelView( mRotation );
    
    gl::lineWidth(1.0f);
    gl::color(ColorAf(1.0f,1.0f,1.0f,0.5f));
    gl::enableWireframe();
    gl::draw( mVBO );
    gl::disableWireframe();
    gl::popMatrices();

    gl::disableAlphaBlending();
    
	// unbind the framebuffer, so that drawing goes to the screen again
	mFBO.unbindFramebuffer();

}

ci::Vec2i TankContent::getSize()
{
    return ci::Vec2i(FBO_WIDTH, FBO_HEIGHT);
}

ci::gl::Texture TankContent::getTexture()
{
    return mFBO.getTexture(0);
}
