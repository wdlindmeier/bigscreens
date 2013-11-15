#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Shader.h"
#include "cinder/Camera.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderProjectApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    CameraPersp mCam;
    
    float mTankDirectionRadians;
};

void CinderProjectApp::setup()
{
    mCam.setPerspective(45, getWindowAspectRatio(), 0.1, 100000);
    mTankDirectionRadians = 0;
}

void CinderProjectApp::mouseDown( MouseEvent event )
{
    mTankDirectionRadians = 0;
}

void CinderProjectApp::update()
{
}

float HeightForXZ(float x, float z)
{
    const static float kDepthMulti = 0.45;
    return (z + x) * kDepthMulti;
}

void CinderProjectApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    Vec3f camEye(0,0,500);
    Vec3f camTarget = Vec3f::zero();
    
    //mCam.setLensShift(Vec2f(0,-0.35));
    mCam.lookAt(camEye, camTarget);
    
    gl::setMatrices(mCam);
    
    gl::bindStockShader(gl::ShaderDef().color());
    
    gl::color(ColorAf(1,0,0,1));
    gl::setDefaultShaderVars();
    
    Vec3f scene(getWindowWidth() * 0.5, getWindowHeight() * 0.5, getWindowHeight() * 0.5);
    
    // Center
    // gl::translate(scene);
    
    float xPosLeft = scene.x * -0.5;
    float xPosRight = scene.x * 0.5;
    float zPosRear = scene.z * -0.5;
    float zPosFore = scene.z * 0.5;
    
    // Simple function so we can get height from any point in the scene:
    float heightBL = HeightForXZ(xPosLeft, zPosRear);
    float heightFL = HeightForXZ(xPosLeft, zPosFore);
    float heightFR = HeightForXZ(xPosRight, zPosFore);
    float heightBR = HeightForXZ(xPosRight, zPosRear);
    
    float xMid = (xPosLeft + xPosRight) / 2.0f;
    float zMid = (zPosRear + zPosFore) / 2.0f;
    
    // Back Left
    gl::pushMatrices();
    gl::color(ColorAf(1,0,0,1));
    gl::translate(Vec3f(-5,-5,zPosRear));
    gl::drawSolidRect(Rectf(xPosLeft,
                            heightBL,
                            xPosLeft + 10,
                            heightBL + 10));
    gl::popMatrices();
    
    // Front Left
    gl::pushMatrices();
    gl::color(ColorAf(0,1,0,1));
    gl::translate(Vec3f(-5,-5,zPosFore));
    gl::drawSolidRect(Rectf(xPosLeft,
                            heightFL,
                            xPosLeft + 10,
                            heightFL + 10));
    gl::popMatrices();

    // Front Right
    gl::pushMatrices();
    gl::color(ColorAf(1,0,1,1));
    gl::translate(Vec3f(-5,-5,zPosFore));
    gl::drawSolidRect(Rectf(xPosRight,
                            heightFR,
                            xPosRight + 10,
                            heightFR + 10));
    gl::popMatrices();

    // Back Right
    gl::pushMatrices();
    gl::color(ColorAf(1,1,1,1));
    gl::translate(Vec3f(-5,-5,zPosRear));
    gl::drawSolidRect(Rectf(xPosRight,
                            heightBR,
                            xPosRight + 10,
                            heightBR + 10));
    gl::popMatrices();

    
    mTankDirectionRadians += 0.003;
    
    Vec2f tankPosRear(xMid, zPosRear);
    Vec2f tankPosFore(xMid, zPosFore);
    Vec2f tankPosLeft(xPosLeft, zMid);
    Vec2f tankPosRight(xPosRight, zMid);
    
    const static float kTankY = -20;
    
    // Rotate arond Zero depending upon the direction
    if (mTankDirectionRadians != 0)
    {
        tankPosRear.rotate(-mTankDirectionRadians);
        tankPosFore.rotate(-mTankDirectionRadians);
        tankPosLeft.rotate(-mTankDirectionRadians);
        tankPosRight.rotate(-mTankDirectionRadians);
    }
    
    // Rear sample pos
    gl::drawCube(Vec3f(tankPosRear.x, kTankY + 20, tankPosRear.y),
                 Vec3f(10,10,10));
    // Front sample pos
    gl::drawCube(Vec3f(tankPosFore.x, kTankY + 20, tankPosFore.y),
                 Vec3f(10,10,10));
    // Left sample pos
    gl::drawCube(Vec3f(tankPosLeft.x, kTankY + 20, tankPosLeft.y),
                 Vec3f(10,10,10));
    // Right sample pos
    gl::drawCube(Vec3f(tankPosRight.x, kTankY + 20, tankPosRight.y),
                 Vec3f(10,10,10));
    
    // HERES THE MATH
    float heightRear = HeightForXZ(tankPosRear.x, tankPosRear.y);
    float heightFore = HeightForXZ(tankPosFore.x, tankPosFore.y);
    float heightLeft = HeightForXZ(tankPosLeft.x, tankPosLeft.y);
    float heightRight = HeightForXZ(tankPosRight.x, tankPosRight.y);
    
    float height = (heightRear + heightFore + heightLeft + heightRight) / 4.f;
    
    // Get the X Angle
    float zOffset = zPosFore - zPosRear;
    float yOffset = heightFore - heightRear;
    float radsAngleX = atan2f(zOffset, yOffset);
    
    // Get the Z Angle
    float xOffset = xPosRight - xPosLeft;
    yOffset = heightLeft - heightRight;
    float radsAngleZ = atan2f(xOffset, yOffset);

    gl::pushMatrices();
    
    gl::rotate(toDegrees(mTankDirectionRadians), 0, 1, 0);

    // Adjust the height
    gl::translate(Vec3f(0, height, 0));
    
    // Adjust the X angle
    gl::rotate(toDegrees(radsAngleX) - 90, 1, 0, 0);
    
    // Adjsut the Z angle
    gl::rotate(toDegrees(radsAngleZ) - 90, 0, 0, 1);

    gl::drawCube(Vec3f(0, 0, 0),
                 Vec3f(200,10,200));
    
    gl::popMatrices();

    
}

CINDER_APP_NATIVE( CinderProjectApp, RendererGl )
