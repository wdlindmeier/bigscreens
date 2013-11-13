#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/gl/Shader.h"

#include "DumbTank.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicTankFeedBackApp : public AppNative {
  public:
	void setup();
	void mouseDrag( MouseEvent event );
	void update();
	void draw();
	
	bigscreens::DumbTankRef mDumbTank;
	CameraPersp				mCam;
	Vec3f					seperationPoint;
	int						tankseperation;
};

void BasicTankFeedBackApp::setup()
{
	mDumbTank = bigscreens::DumbTankRef( new bigscreens::DumbTank() );
	mCam.setPerspective( 60, getWindowAspectRatio(), .1, 100000 );

	mCam.lookAt( Vec3f( 0, 0, 800 ), Vec3f( 0, 0, 0 ) );
	seperationPoint.set( 0, 107, 470 );
}

void BasicTankFeedBackApp::mouseDrag( MouseEvent event )
{
//	mDumbTank->loadShader();
	tankseperation = ((float)event.getPos().x / getWindowWidth()) * 100;
}

void BasicTankFeedBackApp::update()
{
	static int i = 1;
	seperationPoint.z-=i;
	if( seperationPoint.z < -400 || seperationPoint.z > 470)
		i*=-1;

}

void BasicTankFeedBackApp::draw()
{
	static int i = 0;
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	gl::pushMatrices();
	gl::setMatrices( mCam );
		// Turn on y radius
		gl::multModelView( Matrix44f::createRotation( Vec3f( 0, 1, 0), toRadians( (float)i++ ) ) );
		// Bring both the circle down and the tank down
		gl::multModelView( Matrix44f::createTranslation( Vec3f( 0, -107, 0 ) ) );
		
		// Quick shoder for drawSolidCircle
		gl::bindStockShader( gl::ShaderDef().color() );
		gl::pushModelView();
			// Translate the circle to the front of the barrel (front is x: 0, y: 107 z: 470) ( back is z: -400 )
			gl::multModelView( Matrix44f::createTranslation( seperationPoint ) );
				
				gl::drawSolidCircle( Vec2f( 0, 0 ), toRadians( (float)360 ) );
		gl::popModelView();
	
		mDumbTank->draw( tankseperation, seperationPoint );
	gl::popMatrices();
}

CINDER_APP_NATIVE( BasicTankFeedBackApp, RendererGl )
