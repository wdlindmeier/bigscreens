#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Vbo.h"
#include "cinder/TriMesh.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "FloorPlane.h"

using namespace ci;
using namespace ci::app;
using namespace std;
          // how many across, how many up,  how many units between, element number to create the quads
          //                                                        this was the magic sauce for the elements
const int    xCount = 50,    yCount = 50, quadSize = 10,          indexNum = (xCount-1)*(yCount-1) * 4;

class ADJACENCYApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    void loadShaders();
	void createQuad();
	void prepareSettings( Settings * settings ) { /*settings->setWindowSize( 1600, 1600 ); */ }
	void keyDown( KeyEvent event );

    CameraPersp		mCam;
	gl::FboRef		mFbo;
	
	// fbo effects
	gl::VaoRef		mBillboardVao;
	gl::VboRef		mBillboardVbo, mBillboardElementVbo;
	gl::GlslProgRef	mEffectsGlsl;
	gl::Fbo::Format mFboFormat;
	
	bigscreens::FloorPlaneRef mFloorPlane;
};

void ADJACENCYApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'f') {
		setFullScreen(true);
		gl::viewport( Vec2i( 0, 0 ), getWindowSize() );
		mFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight(), mFboFormat );
	}
	else if( event.getCode() == KeyEvent::KEY_ESCAPE ) {
		setFullScreen( false );
		gl::viewport( Vec2i(), getWindowSize() );
		mFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight(), mFboFormat );
	}
}

void ADJACENCYApp::setup()
{
	
	mFloorPlane = bigscreens::FloorPlaneRef( new bigscreens::FloorPlane() );
    
	mFboFormat.colorTexture().depthBuffer();
	mFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight(), mFboFormat );
	
    mCam.setPerspective( 60, getWindowWidth() / getWindowHeight(), .1, 10000 );
    mCam.lookAt( Vec3f( 0, 0, 1 ), Vec3f( 0, 0, 0 ) );
    
    loadShaders();
	createQuad();
}

void ADJACENCYApp::loadShaders()
{
	// Scanline Stuff
	gl::GlslProg::Format mEffectsFormat;
	mEffectsFormat.vertex( loadAsset( "finalEffects.vert" ) )
	.fragment( loadAsset( "finalEffects.frag" ) );
	mEffectsGlsl = gl::GlslProg::create( mEffectsFormat );
}

void ADJACENCYApp::createQuad()
{
	float vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f
	};
	
	uint32_t index[] = {
		0, 1, 2,
		0, 2, 3
	};
	
	mBillboardVao = gl::Vao::create();
	mBillboardVao->bind();
	
	mBillboardVbo = gl::Vbo::create( GL_ARRAY_BUFFER, 20 * sizeof(float), vertices, GL_STATIC_DRAW );
	mBillboardVbo->bind();
	
	gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	gl::enableVertexAttribArray(0);
	gl::vertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)) );
	gl::enableVertexAttribArray(1);
	
	mBillboardElementVbo = gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof( uint32_t ), index, GL_STATIC_DRAW );
	
}

void ADJACENCYApp::mouseDown( MouseEvent event )
{
	
}

void ADJACENCYApp::update()
{
	
}

void ADJACENCYApp::draw()
{
	
	static int index = 0;
	static float divideNum = 0.0f;
	
	mFbo->bindFramebuffer();
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::pushMatrices();
    gl::setMatrices( mCam );
    // simple centering
    gl::multModelView( Matrix44f::createTranslation( Vec3f( -((xCount-1)*quadSize) / 2, -((yCount-1)*quadSize) / 4, (-((xCount-1) * quadSize) / 8) )   ) );
	gl::multModelView( Matrix44f::createRotation( Vec3f( 1, 0, 0 ), toRadians( -80.0f ) ) );
//	gl::multModelView( Matrix44f::createScale( Vec3f( .05, 0, 0 ) ) );
    
//	cout << "modelview in app: " << gl::getModelView() << endl;
	
	mFloorPlane->draw();
	
	
    gl::popMatrices();
    
	mFbo->unbindFramebuffer();
	
	
	gl::pushMatrices();
	
	mBillboardVao->bind();
	mBillboardElementVbo->bind();
	mFbo->bindTexture();
	
	mEffectsGlsl->bind();
	mEffectsGlsl->uniform( "fboTexture", 0 );
	mEffectsGlsl->uniform( "texSize", mFbo->getTexture()->getSize() );
	mEffectsGlsl->uniform( "time", (float)getElapsedSeconds() );
	
	gl::drawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
	
	mEffectsGlsl->unbind();
	
	mFbo->unbindTexture();
	mBillboardElementVbo->bind();
	mBillboardVao->bind();
	
	gl::popMatrices();
//
//	gl::bindStockShader( gl::ShaderDef().color().texture() );
//	gl::draw(mFbo->getTexture(), Vec2i(0, 0));
}

CINDER_APP_NATIVE( ADJACENCYApp, RendererGl )
