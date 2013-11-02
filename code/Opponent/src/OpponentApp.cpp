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
	void loadShaders();
	void setupBuffers();
	void createDynamicGeometry();
	void setupParticleBuffer();
	
	
	gl::VboRef		mPositionVbo, mNormalVbo, mElementVbo;
	gl::GlslProgRef	mGlsl;
	CameraPersp		mCam;
	TriMeshRef		mDynamicTrimesh;
	bigscreens::FinalBillboardRef mFinalBillboard;
	gl::FboRef		mFbo;
	gl::TextureRef  mFboTexture;
	
	bool			sphere;
	bigscreens::SphericalGeometryRef mSphere;
	bigscreens::PyramidalGeometryRef mPyramid;
};

void OpponentApp::setup()
{
	mPyramid = bigscreens::PyramidalGeometryRef( new bigscreens::PyramidalGeometry() );
	mSphere = bigscreens::SphericalGeometryRef( new bigscreens::SphericalGeometry() );
	
	sphere = false;
	
	mFinalBillboard = bigscreens::FinalBillboardRef( new bigscreens::FinalBillboard() );
	
	gl::Fbo::Format mFboFormat;
	mFboFormat.colorTexture().stencilBuffer().
	depthTexture( gl::Texture::Format().internalFormat( GL_DEPTH_COMPONENT32F ) ).samples(16);
	mFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight(), mFboFormat );

	TriMesh::Format mDynamicTrimeshFormat;
	mDynamicTrimeshFormat.vertices(3).normals();
	
	
	mDynamicTrimesh = TriMesh::create( mDynamicTrimeshFormat );
	
	setupBuffers();
	loadShaders();
	
	mCam.setPerspective( 60, getWindowWidth() / getWindowHeight(), .01, 1000 );
	mCam.lookAt( Vec3f( 0, 0, 1 ), Vec3f( 0, 0, 0 ) );
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void OpponentApp::setupBuffers()
{
}

void OpponentApp::setupParticleBuffer()
{
//	Vec3f v(0.0f);
//	float velocity, theta, phi;
//	GLfloat *data = new GLfloat[nParticles * 3];
//	for( GLuint i = 0; i<nParticles; i++ ) {
//		// Pick the direction of the velocity
//		theta = glm::mix(0.0f, (float)PI / 6.0f, randFloat());
//		phi = glm::mix(0.0f, (float)TWOPI, randFloat());
//		v.x = sinf(theta) * cosf(phi);
//		v.y = cosf(theta);
//		v.z = sinf(theta) * sinf(phi);
//		// Scale to set the magnitude of the velocity (speed)
//		velocity = glm::mix(1.25f,1.5f,randFloat());
//		v = v * velocity;
//		data[3*i]   = v.x;
//		data[3*i+1] = v.y;
//		data[3*i+2] = v.z;
//	}
//	glBindBuffer(GL_ARRAY_BUFFER,initVel);
//	glBufferSubData(GL_ARRAY_BUFFER, 0,
//					nParticles * 3 * sizeof(float), data);
}

void OpponentApp::createDynamicGeometry()
{
	Vec3f * positions = new Vec3f[TOTAL_POSITIONS];
	Vec3f * normals = new Vec3f[TOTAL_POSITIONS];

	positions[0] = Vec3f( -4, -5, 0 );
	positions[1] = Vec3f( 4, -5, 0 );
	positions[2] = Vec3f( 0, 5, 0 );
	positions[3] = Vec3f( 0, 0, 0 );
	
	int i = 4;
	for( int z = 0; z < 10; z++ ) {
		for( int y = 0; y < 10; y++ ) {
			for( int x = 0; x < 10; x++ ) {
				positions[i] = Vec3f( sin(i) * (x + 10), cos(i) * (y + 1), sin(i) * cos(i) * (z + 1));
				i++;
			}
		}
	}
	
	mDynamicTrimesh->appendVertices( positions, TOTAL_POSITIONS );
	
	cout << i << endl;
	uint32_t * index = new uint32_t[TOTAL_INDICES];
	
	index[0] = 0;
	index[1] = 1;
	index[2] = 2;
	
	i = 3;
	for( int j = 3; j < TOTAL_INDICES; j+=3 ) {
		index[j+0] = 3;
		index[j+1] = i+1;
		index[j+2] = i+2;
		i+=3;
	}
	
	mDynamicTrimesh->appendIndices( index, TOTAL_INDICES );
	mDynamicTrimesh->recalculateNormals();
	
	delete [] positions;
	delete [] index;
	delete [] normals;
}

void OpponentApp::loadShaders()
{
	gl::GlslProg::Format mFormat;
	mFormat.vertex( loadAsset( "triangle.vert" ) ).fragment( loadAsset( "triangle.frag" ) );
	mGlsl = gl::GlslProg::create( mFormat );
}

void OpponentApp::mouseDown( MouseEvent event )
{
	mFinalBillboard->loadShaders();
	sphere = !sphere;
}

void OpponentApp::update()
{
	static int i = 0;
	mFbo->bindFramebuffer();
	gl::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	gl::pushMatrices();
	gl::setMatrices( mCam );
	gl::multModelView( Matrix44f::createTranslation( Vec3f( 0, 0, -1 ) ) );
	gl::multModelView( Matrix44f::createRotation( Vec3f( 0, 1, 0 ), toRadians( (float)i++ ) ) );
	
	mGlsl->bind();
	
	mGlsl->uniform( "projection", gl::getProjection() );
	mGlsl->uniform( "modelView", gl::getModelView() );
	
	glPointSize(10.0f);
	
	if( sphere ) {
		mSphere->draw();
	}
	else {
		mPyramid->draw();
	}
	
	mGlsl->unbind();
	
	gl::popMatrices();
	mFbo->unbindFramebuffer();
	mFboTexture = mFbo->getTexture();
}

void OpponentApp::draw()
{
	
	// clear out the window with black
	gl::clear();
	
//	gl::draw( mFboTexture, Vec2i( 0, 0 ) );
	mFinalBillboard->draw( mFbo->getTexture() );
}

CINDER_APP_NATIVE( OpponentApp, RendererGl )
