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

    CameraPersp         mCam;
    float               mCameraRotation;
    
};

void OBJTestApp::setup()
{
    mTank.load();
    
    // Cam
    mCam.setPerspective( 15.0f, (float)getWindowWidth() / getWindowHeight(), .01, 40000 );
    mCam.lookAt( Vec3f( 0, 0, 0 ), Vec3f( 0, 0, 0 ) );
    
    mCameraRotation = 0.0f;
}

void OBJTestApp::mouseDown( MouseEvent event )
{
    mTank.fire();
}

void OBJTestApp::update()
{
    mCameraRotation -= 0.005f;
    
    float camX = cosf(mCameraRotation) * 100;
    float camZ = sinf(mCameraRotation) * 100;
    float camY = 10;
    
    Vec3f lookAt( 0, 0, 0 );
    // NOTE: Only for "tank.obj"
    camY *= 120;
    camX *= 60;
    camZ *= 60;
    lookAt.y = 100;

    mCam.lookAt( Vec3f( camX, camY, camZ ), lookAt );
}

void OBJTestApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::pushMatrices();
    gl::setMatrices( mCam );

    mTank.render(getElapsedFrames());
    
    gl::popMatrices();
}

CINDER_APP_NATIVE( OBJTestApp, RendererGl )
