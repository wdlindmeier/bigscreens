#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Shader.h"
#include "cinder/Camera.h"

using namespace ci;
using namespace ci::app;
using namespace std;

float mHeightOffset;

class CinderProjectApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );
    void keyDown(KeyEvent event);
    void mouseDrag(cinder::app::MouseEvent event);
	void update();
	void draw();
    void drawGun();
    void drawShot();
    void createShotMat();
    
    CameraPersp mCam;
    
    float mTankDirectionRadians;
    float mBarrelDirectionRadians;
    float mShotProgress;
    
    Matrix44f mTankMat;
    Matrix44f mGunMat;
    
    Matrix44f mShotTankMat;
    Matrix44f mShotGunMat;
};

void CinderProjectApp::setup()
{
    mCam.setPerspective(45, getWindowAspectRatio(), 0.1, 100000);
    mTankDirectionRadians = 0;
    mBarrelDirectionRadians = 0;
    
    mHeightOffset = 0;

    Vec3f camEye(0,0,500);//500);
    Vec3f camTarget = Vec3f::zero();
    //mCam.setLensShift(Vec2f(0,-0.35));
    mCam.lookAt(camEye, camTarget);
    
    mShotProgress = 0;
}

void CinderProjectApp::keyDown(KeyEvent event)
{
    if (event.getChar() == ' ')
    {
        mShotTankMat = mTankMat;
        mShotGunMat = mGunMat;
        mShotProgress = 0;
    }
}

void CinderProjectApp::mouseDrag(cinder::app::MouseEvent event)
{
    float xTransition = (event.getPos().x / (float)getWindowWidth()) * M_PI;
    float yTransition = (event.getPos().y / (float)getWindowHeight()) * M_PI;
    
    mHeightOffset = (getWindowHeight() * yTransition) - (getWindowHeight() * 0.5);
    
    Vec3f camEye(cos(xTransition) * 500,
                 0,// 1000.0f * (event.getPos().y / (float)getWindowHeight()) - 500.0f,
                 sin(xTransition) * 500);//500);
    Vec3f camTarget = Vec3f::zero();
    //mCam.setLensShift(Vec2f(0,-0.35));
    mCam.lookAt(camEye, camTarget);
}

void CinderProjectApp::mouseDown( MouseEvent event )
{
//    mTankDirectionRadians = 0;
//    mBarrelDirectionRadians = 0;
}

void CinderProjectApp::update()
{
    mShotProgress += 0.1;
}

float HeightForXZ(float x, float z)
{
    const static float kDepthMulti = 0.45;
    return mHeightOffset + (z + x) * kDepthMulti;
}

static const float kBarrelYOffset = 100;
static const float kBarrelAngleDegrees = -15;
static const float kBarrelLength = 200;

// Calculate Shot Mats
const static double kGravity = 9.81;
const static Vec3f shotAxis(0,0,1);
const static Vec3f angleAxis(1,0,0);
const static Vec3f spinAxis(0,1,0);

void CinderProjectApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    
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
    

    // Rotate arond Zero depending upon the direction
    if (mTankDirectionRadians != 0)
    {
        tankPosRear.rotate(-mTankDirectionRadians);
        tankPosFore.rotate(-mTankDirectionRadians);
        tankPosLeft.rotate(-mTankDirectionRadians);
        tankPosRight.rotate(-mTankDirectionRadians);
    }
    
    /*
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
    */
    
    // HERES THE MATH
    float heightRear = HeightForXZ(tankPosRear.x, tankPosRear.y);
    float heightFore = HeightForXZ(tankPosFore.x, tankPosFore.y);
    float heightLeft = HeightForXZ(tankPosLeft.x, tankPosLeft.y);
    float heightRight = HeightForXZ(tankPosRight.x, tankPosRight.y);
    
    float tankHeight = (heightRear + heightFore + heightLeft + heightRight) / 4.f;
    
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
    gl::translate(Vec3f(0, tankHeight, 0));
    
    // Adjust the X angle
    gl::rotate(toDegrees(radsAngleX) - 90, 1, 0, 0);
    
    // Adjsut the Z angle
    gl::rotate(toDegrees(radsAngleZ) - 90, 0, 0, 1);

    gl::drawCube(Vec3f(0, 0, 0),
                 Vec3f(200,10,200));
    
    drawGun();

    gl::popMatrices();
    
    mTankMat = Matrix44f::identity();
    
    // Tank Mat
    // TAKEN FROM ABOVE
    // Adjust to tank orientation
    mTankMat.rotate(Vec3f(0, 1, 0), mTankDirectionRadians);
    // Adjust the height
    mTankMat.translate(Vec3f(0, tankHeight, 0));
    // X & Y angles
    mTankMat.rotate(Vec3f(1, 0, 0), radsAngleX - (M_PI * 0.5));
    mTankMat.rotate(Vec3f(0, 0, 1), radsAngleZ - (M_PI * 0.5));
    // END TAKEN FROM ABOVE

    // Gun Mat
    mGunMat = Matrix44f::identity();
    // Barrel Spin rotation
    mGunMat.rotate(spinAxis, mBarrelDirectionRadians);
    // Raise to barrel height
    mGunMat.translate(Vec3f(0,kBarrelYOffset,0));
    // Barrel shot angle (-50)
    float barrelTheta = toRadians(kBarrelAngleDegrees);
    mGunMat.rotate(angleAxis, barrelTheta);
    // Get the distance
    float x = cos(barrelTheta) * kBarrelLength;
    float y = sin(barrelTheta) * kBarrelLength;
    float dist = kBarrelLength;//sqrt(x*x + y*y);
    // Barrel length (tip) (on Z axis)
    mGunMat.translate(shotAxis * dist);
    
    drawShot();
}

void CinderProjectApp::drawShot()
{
    // Draw the shot
    
//    mShotTankMat = mTankMat;
//    mShotGunMat = mGunMat;

    Matrix44f tankGunTransformMat = mShotTankMat * mShotGunMat;

    Vec3f shotOrigin = tankGunTransformMat.transformPoint(Vec3f(0,0,0));

    float velocity = 100;

    Vec3f barrelBase = Vec3f(0,kBarrelYOffset,0);
    barrelBase = mShotTankMat.transformPoint(barrelBase);
    gl::drawCube(barrelBase, Vec3f(10, 10, 10));
    
    Vec3f relativeOrigin = shotOrigin - barrelBase;
    float roY = relativeOrigin.y;
    float roX = Vec3f(relativeOrigin.x, 0, relativeOrigin.z).distance(Vec3f::zero());
    float shotTheta = atan2(roY, roX);
    
    float shotY = velocity*sin(shotTheta)*mShotProgress;
    float shotX = velocity*cos(shotTheta)*mShotProgress;
    // Converts the X to whatever the shot axis is
    Vec3f shotPos = Vec3f(0,shotY,0) + (Vec3f(shotX,shotX,shotX) * shotAxis);

    shotPos += Vec3f(0,roY,roX);
    
    float relativeAngle = atan2(relativeOrigin.z, relativeOrigin.x);
    shotPos.rotate(Vec3f(0,1,0), -relativeAngle + (M_PI * 0.5));
    
    Vec3f worldPos = barrelBase + shotPos;
    worldPos.y -= 0.5*kGravity*mShotProgress*mShotProgress;
    
    gl::drawCube(shotOrigin,
                 Vec3f(20,20,20));
    
    gl::drawCube(worldPos,
                 Vec3f(10,10,10));

}

void CinderProjectApp::drawGun()
{
    mBarrelDirectionRadians += 0.01;
    
    // return;
    
    gl::pushMatrices();
    gl::translate(Vec3f(0, kBarrelYOffset, 0));
    
    // Pivot point
    // gl::drawCube(Vec3f(0, 0, 0), Vec3f(10, 10, 10));

    // Direction
    gl::rotate(toDegrees(mBarrelDirectionRadians), 0, 1, 0);
    
    // Angle
    gl::rotate(kBarrelAngleDegrees, 1, 0, 0);

    Vec3f barrelSize(20, 20, kBarrelLength);
    gl::drawCube(Vec3f(0,
                       0,
                       barrelSize.z * 0.5), barrelSize);
    gl::popMatrices();
}

CINDER_APP_NATIVE( CinderProjectApp, RendererGl )
