#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/ObjLoader.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"
#include "cinder/Rand.h"
#include "AdvancedTank.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace bigscreens;

class OBJTestApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    AdvancedTank mTank;
    
    /*
    void loadShader();
    void loadModel();
    
    ci::TriMeshRef		mTankMesh;
    ci::gl::VboRef		mTankVbo;
    ci::gl::VboRef		mTankElementVbo;
    ci::gl::VaoRef		mTankVao;
    ci::gl::GlslProgRef mTankShader;
    */
    
    CameraPersp         mCam;
    float               mCameraRotation;
    
};

void OBJTestApp::setup()
{
    /*
    loadShader();
    loadModel();
    */
    
    mTank.load();
    
    // Cam
    mCam.setPerspective( 15.0f, (float)getWindowWidth() / getWindowHeight(), .01, 40000 );
    mCam.lookAt( Vec3f( 0, 0, 0 ), Vec3f( 0, 0, 0 ) );
    
    mCameraRotation = 0.0f;
}
/*
void OBJTestApp::loadShader()
{
    gl::GlslProg::Format mFormat;
    // NOTE: These must be resorces, not assets
    mFormat.vertex( loadResource( "basic.vert" ) )
    .fragment( loadResource( "basic.frag" ) );
    mTankShader = gl::GlslProg::create( mFormat );
    mTankShader->bind();
}

void OBJTestApp::loadModel()
{
    DataSourceRef file = loadResource( "tank.obj" ); //"uav.obj" // "atat.obj"
    ObjLoader loader( file );
    mTankMesh = TriMesh::create( loader );
    
    mTankVao = gl::Vao::create();
    mTankVao->bind();
    
    mTankVbo = gl::Vbo::create(GL_ARRAY_BUFFER, 3 * mTankMesh->getNumVertices() * sizeof(float),
                               mTankMesh->getVertices<3>(), GL_STATIC_DRAW );
    mTankVbo->bind();
    
    GLint pos = mTankShader->getAttribLocation( "vPosition" );
    gl::enableVertexAttribArray( pos );
    gl::vertexAttribPointer( pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    mTankElementVbo = gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER,
                                      mTankMesh->getNumIndices() * 4,
                                      mTankMesh->getIndices().data());
    mTankElementVbo->bind();
    mTankElementVbo->unbind();
    
    mTankVbo->unbind();
    mTankVao->unbind();
    mTankShader->unbind();
    
//    mVboMesh = ci::gl::VboMesh::create(*mTankMesh);
}
*/
void OBJTestApp::mouseDown( MouseEvent event )
{
}

void OBJTestApp::update()
{
    mCameraRotation -= 0.01f;
    
    float camX = cosf(mCameraRotation) * 100;
    float camZ = sinf(mCameraRotation) * 100;
    float camY = 10;
    
    Vec3f lookAt( 0, 0, 0 );
    // NOTE: Only for "tank.obj"
    camY *= 60;
    camX *= 30;
    camZ *= 30;
    lookAt.y = 100;

    mCam.lookAt( Vec3f( camX, camY, camZ ), lookAt );
    
    mTank.update();
}

void OBJTestApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::pushMatrices();
    gl::setMatrices( mCam );

    mTank.render(Vec2f::zero());
    
    gl::popMatrices();

    
    /*
    gl::enableAdditiveBlending();
    
    mTankShader->bind();
    gl::pushMatrices();
    gl::setMatrices( mCam );
    
    gl::setDefaultShaderVars();

    mTankVao->bind();
    mTankElementVbo->bind();
    
    int numIndices = mTankMesh->getNumIndices();

    mTankShader->uniform("uColor", ColorAf(0,1,1,0.05f));
    gl::drawElements(GL_LINES,
                     numIndices,
                     GL_UNSIGNED_INT,
                     0);

    mTankShader->uniform("uColor", ColorAf(1,1,1,1.0f));
    float scalarCompleteness = sin(getElapsedFrames() * 0.01);
    int frameCount = scalarCompleteness * numIndices;
    frameCount = frameCount - (frameCount % 2);
    int count = std::min<int>(frameCount, numIndices);
    int offset = Rand::randInt(numIndices - count);
    offset = offset - (offset % 2);
    gl::drawElements(GL_LINES,
                     count,
                     GL_UNSIGNED_INT,
                     (void*)(offset * sizeof(GLuint)));

    mTankElementVbo->unbind();
    mTankVao->unbind();
     
    mTankShader->unbind();
    gl::popMatrices();
    */
}

CINDER_APP_NATIVE( OBJTestApp, RendererGl )
