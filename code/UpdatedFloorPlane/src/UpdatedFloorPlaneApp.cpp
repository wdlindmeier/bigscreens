#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"

#include "FloorPlane.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class UpdatedFloorPlaneApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void update();
	void draw();
	
	bigscreens::FloorPlaneRef	mFloorPlane;
	CameraPersp					mCam;
};

void UpdatedFloorPlaneApp::setup()
{
	mFloorPlane = bigscreens::FloorPlaneRef( new bigscreens::FloorPlane() );
	
	mCam.setPerspective( 60, getWindowAspectRatio(), .1, 1000 );
	mCam.lookAt( Vec3f( 0, 0, 50 ), Vec3f( 0, 0, 0 ) );
}

void UpdatedFloorPlaneApp::mouseDown( MouseEvent event )
{
	mFloorPlane->toggleDrawColoredQuads();
}

void UpdatedFloorPlaneApp::mouseDrag( MouseEvent event )
{
	mFloorPlane->setNearLimit( ((float)event.getPos().y / getWindowHeight()) * 500 );
	mFloorPlane->setFarLimit( ((float)event.getPos().x / getWindowWidth()) * 500 );
}

void UpdatedFloorPlaneApp::update()
{
}

void UpdatedFloorPlaneApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	gl::pushMatrices();
	gl::setMatrices( mCam );
	gl::multModelView( Matrix44f::createTranslation( Vec3f( -250, -100, -200 ) ) );
	gl::multModelView( Matrix44f::createRotation( Vec3f( 0, 1, 0 ), toRadians( 90.0f ) ) );
	
	mFloorPlane->draw( getElapsedFrames() );
	
	gl::popMatrices();
}

CINDER_APP_NATIVE( UpdatedFloorPlaneApp, RendererGl )
