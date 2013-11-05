#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Camera.h"
#include "cinder/gl/Shader.h"
#include "cinder/TriMesh.h"
#include "cinder/CinderMath.h"
#include "FinalBillboard.h"
#include "cinder/gl/Fbo.h"
#include "PyramidalGeometry.hpp"
#include "SphericalGeometry.hpp"
#include "SmokeEffect.hpp"
#include "Opponent.h"

using namespace ci;
using namespace ci::app;
using namespace std;

enum  {
	TOTAL_POSITIONS = 1000,
	TOTAL_INDICES = 333
};

class OpponentApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void createDynamicGeometry();

	gl::VboRef		mPositionVbo, mNormalVbo, mElementVbo;
	CameraPersp		mCam;
	
	gl::FboRef		mFbo;
	gl::TextureRef  mFboTexture;
	
	bigscreens::OpponentRef			mOpponent;
	bigscreens::FinalBillboardRef mFinalBillboard;
};

void OpponentApp::setup()
{
	mOpponent = bigscreens::OpponentRef( new bigscreens::Opponent() );
	
	mFinalBillboard = bigscreens::FinalBillboardRef( new bigscreens::FinalBillboard() );
	
	gl::Fbo::Format mFboFormat;
	mFboFormat.colorTexture().stencilBuffer().
	depthTexture( gl::Texture::Format().internalFormat( GL_DEPTH_COMPONENT32F ) ).samples(16);
	mFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight(), mFboFormat );
	
	mCam.setPerspective( 60, getWindowWidth() / getWindowHeight(), .01, 1000 );
	mCam.lookAt( Vec3f( 0, 0, 1 ), Vec3f( 0, 0, 0 ) );
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void OpponentApp::mouseDown( MouseEvent event )
{
	mFinalBillboard->loadShaders();
}

void OpponentApp::update()
{
	static int i = 0;
	mFbo->bindFramebuffer();
	
	ci::gl::pushMatrices();
	ci::gl::setMatrices( mCam );
	ci::gl::multModelView( Matrix44f::createTranslation( Vec3f( 0, 0, -1 ) ) );
	ci::gl::multModelView( Matrix44f::createRotation( Vec3f( 0, 1, 0 ), toRadians( (float)i++ ) ) );
	
	mOpponent->draw();
	
	mFbo->unbindFramebuffer();
	mFboTexture = mFbo->getTexture();
}

void OpponentApp::draw()
{
	
	// clear out the window with black
	gl::clear();
	
	mFinalBillboard->draw( mFboTexture );
}

CINDER_APP_NATIVE( OpponentApp, RendererGl )
