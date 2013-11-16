#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/gl/Fbo.h"
#define USE_DYNAMIC_ASSETS

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
	Vec3f					lastPos, pos;
};

void UpdatedOpponentApp::setup()
{
	mFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight(), gl::Fbo::Format().colorTexture().depthBuffer( GL_DEPTH_COMPONENT32F ) );
	
	mOpponent = bigscreens::OpponentRef( new bigscreens::Opponent() );
	mMinion = bigscreens::MinionGeometryRef( new bigscreens::MinionGeometry() );
	mFinalBillboard = bigscreens::FinalBillboardRef( new bigscreens::FinalBillboard() );
	
	mCam.setPerspective( 70, getWindowWidth() / getWindowHeight(), .01, 1000 );
	mCam.lookAt( Vec3f( 0, 0, 1 ), Vec3f( 0, 0, 0 ) );
	
	pos = lastPos = Vec3f( 0, 0, 0 );
	rotation = lightY = lightX = 0;
	percentage = 0.6f;
	zDepth = -5.0f;
	pos.z = zDepth;
	
	mFbo->bindFramebuffer();
	gl::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	mFbo->unbindFramebuffer();
	
}

void UpdatedOpponentApp::mouseDown(cinder::app::MouseEvent event)
{
//	mOpponent->loadShaders();
}

void UpdatedOpponentApp::mouseDrag( MouseEvent event )
{
	lightX = (((float)event.getPos().x / getWindowWidth()) * 100.0f) - 50.0f;
	lightY = (((float)event.getPos().y / getWindowHeight()) * 100.0f) - 50.0f;
//	percentage = (float)event.getPos().x / getWindowWidth();
	zDepth = ((float)event.getPos().x / getWindowWidth()) * 5;
//	pos.x = ((float)event.getPos().x / getWindowWidth());
//	pos.y = ((float)event.getPos().y / getWindowHeight());
	mOpponent->setUpdateGeometry(true);
	pos.z = -zDepth;
}

void UpdatedOpponentApp::update()
{
	mOpponent->update(percentage, (pos - lastPos) * 100 );
	lastPos = pos;
}

void UpdatedOpponentApp::draw()
{
	
	mFbo->bindFramebuffer();
	gl::clear( Color( 0, 0, 0 ), true );
	gl::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	
	gl::pushMatrices();
	
	gl::setMatrices( mCam );
	gl::multModelView( Matrix44f::createTranslation( pos ) );
//	gl::multModelView( Matrix44f::createRotation( Vec3f( 0, 1, 0 ), toRadians( (float)rotation++ ) ) );
	
//	mMinion->draw( Vec3f( lightX, lightY, 1 ), ColorA( 1.0, 1.0, 0.0, 1.0 ) );
	mOpponent->draw( zDepth, Vec3f( lightX, lightY, 1 ) );
	
	gl::popMatrices();
	mFbo->unbindFramebuffer();
//
	mFinalBillboard->draw( mFbo->getTexture() );
}

CINDER_APP_NATIVE( UpdatedOpponentApp, RendererGl )
