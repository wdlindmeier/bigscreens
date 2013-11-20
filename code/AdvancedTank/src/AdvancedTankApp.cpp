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
#include "FloorPlane.h"
#include "FloorPlane.h"
#include "DumbTank.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace bigscreens;

class OBJTestApp : public AppNative
{
  public:
    
	void setup();
	void mouseDown( MouseEvent event );
    void keyUp(KeyEvent event);
	void update();
	void draw();
    void drawGround();
    
    AdvancedTankRef     mTank;

    CameraPersp         mCam;
    float               mCameraRotation;
    
    bool                mRotate;
    
    FloorPlaneRef       mFloorPlane;
    Vec3f               mGroundScale;
    
    DumbTankRef         mDumbTank;
};

void OBJTestApp::setup()
{
    setFullScreen(true);
    
    mTank = AdvancedTankRef(new AdvancedTank());
    
    mDumbTank = DumbTankRef(new DumbTank("tank_angled.obj", -18));
    
    // Cam
    mCam.setPerspective( 15.0f, (float)getWindowWidth() / getWindowHeight(), .01, 40000 );
    mCam.lookAt( Vec3f( 0, 0, 0 ), Vec3f( 0, 0, 0 ) );
    
    mRotate = true;
    mCameraRotation = 0.0f;
    
    mFloorPlane = FloorPlaneRef(new FloorPlane(Vec2i(50,50)));
    
    mGroundScale = Vec3f(5000,100,5000);
}

void OBJTestApp::mouseDown( MouseEvent event )
{
    mTank->setFrameContentID(1);
    PositionOrientation po;
    GroundOrientaion go;
    mTank->fire(po,go);
}

void OBJTestApp::keyUp(KeyEvent event)
{
    if (event.getChar() == ' ')
    {
        mRotate = !mRotate;
    }
}

void OBJTestApp::update()
{
    if (mRotate)
    {
        mCameraRotation -= 0.005f;
    }
    
    float camX = cosf(mCameraRotation) * 100;
    float camZ = sinf(mCameraRotation) * 100;
    float camY = 10;
    
    Vec3f lookAt( 0, 0, 0 );
    // NOTE: Only for "tank.obj"
    camY *= 120 * 0.65;
    camX *= 60 * 0.65;
    camZ *= 60 * 0.65;
    lookAt.y = 100;

    mCam.lookAt( Vec3f( camX, camY, camZ ), lookAt );
}

void OBJTestApp::drawGround()
{
    gl::pushMatrices();
    gl::setMatrices( mCam );
    // Scale to taste
    gl::scale(mGroundScale);
    // Center
    gl::translate(Vec3f(-0.5, 0, -0.5));
    
    //mGroundPlane->setNoiseTexture(heightMap);
    
    //const static bool kDrawColorGround = false;
    //mGroundPlane->draw(mNumFramesRendered, kDrawColorGround, ColorAf(0.5,0.5,0.5,mRenderAlpha));
    
    mFloorPlane->draw(getElapsedFrames(), false, ColorAf(0.5,0.5,0.5,1));

    gl::popMatrices();
}

void OBJTestApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    drawGround();
    
    gl::pushMatrices();
    gl::setMatrices( mCam );
    gl::enableAlphaBlending();

    /*
    Vec3f target = Vec3f(cos(getElapsedFrames() * 0.01) * 2000,
                            800,
                            sin(getElapsedFrames() * 0.01) * 2000);
    mTank->setTargetPosition(target);
    mTank->setFrameContentID(1);
    mTank->update(getElapsedFrames());
    //mTank->render(mCam, 1);
    mTank->render(0.75f);
    */
    
    gl::color(ColorAf(1,0,0,1));
    mDumbTank->draw(0, Vec3f::zero());
    
    gl::popMatrices();
}

CINDER_APP_NATIVE( OBJTestApp, RendererGl )
