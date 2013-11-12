#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/gl/Fbo.h"

#include "Opponent.h"
#include "FinalBillboard.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class UpdatedOpponentApp : public AppNative {
  public:
	void setup();
	void mouseDrag( MouseEvent event );
	void mouseDown( MouseEvent event );
	void update();
	void draw();
	
	bigscreens::OpponentRef mOpponent;
	CameraPersp				mCam;
	float					zDepth;
	float					lightX, lightY;
	float					rotation;
	bigscreens::MinionGeometryRef mMinion;
	bigscreens::FinalBillboardRef mFinalBillboard;
	gl::FboRef				mFbo;
	float					percentage;
};

void UpdatedOpponentApp::setup()
{
	mFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight(), gl::Fbo::Format().colorTexture().depthBuffer() );
	mOpponent = bigscreens::OpponentRef( new bigscreens::Opponent() );
	zDepth = 1.5f;
	mCam.setPerspective( 70, getWindowWidth() / getWindowHeight(), .01, 1000 );
	mCam.lookAt( Vec3f( 0, 0, 1 ), Vec3f( 0, 0, 0 ) );
	lightX = 0;
	lightY = 0;
	mMinion = bigscreens::MinionGeometryRef( new bigscreens::MinionGeometry() );
	rotation = 0;
	mFinalBillboard = bigscreens::FinalBillboardRef( new bigscreens::FinalBillboard() );
	
	mFbo->bindFramebuffer();
	gl::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	mFbo->unbindFramebuffer();
	percentage = 0.6f;
}

void UpdatedOpponentApp::mouseDown(cinder::app::MouseEvent event)
{
	mOpponent->loadShaders();
}

void UpdatedOpponentApp::mouseDrag( MouseEvent event )
{
	lightX = (((float)event.getPos().x / getWindowWidth()) * 100.0f) - 50.0f;
	lightY = (((float)event.getPos().y / getWindowHeight()) * 100.0f) - 50.0f;
	percentage = (float)event.getPos().x / getWindowHeight();
}

void UpdatedOpponentApp::update()
{
	mOpponent->update(percentage);
}

void UpdatedOpponentApp::draw()
{
	
	mFbo->bindFramebuffer();
	gl::clear( Color( 0, 0, 0 ), true );
	
	gl::pushMatrices();
	
	gl::setMatrices( mCam );
	gl::multModelView( Matrix44f::createTranslation( Vec3f( 0, 0, -zDepth ) ) );
	gl::multModelView( Matrix44f::createRotation( Vec3f( 0, 1, 0 ), toRadians( (float)rotation++ ) ) );
	
//	mMinion->draw( Vec3f( lightX, lightY, 1 ), ColorA( 1.0, 1.0, 0.0, 1.0 ) );
	mOpponent->draw( zDepth, Vec3f( lightX, lightY, 1 ) );
	
	gl::popMatrices();
	mFbo->unbindFramebuffer();
//
	gl::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	mFinalBillboard->draw( mFbo->getTexture() );
}

CINDER_APP_NATIVE( UpdatedOpponentApp, RendererGl )
