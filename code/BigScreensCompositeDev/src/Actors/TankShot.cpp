//
//  Trajectory.cpp
//  AdvancedTank
//
//  Created by William Lindmeier on 11/3/13.
//
//

#include "TankShot.h"

using namespace ci;
using namespace bigscreens;

// TODO: Make a shared asset manager
static gl::TextureRef ExplosionTexture;

// Calculate Shot Mats
const static Vec3f kShotAxis(0,0,1);
const static Vec3f kAngleAxis(1,0,0);
const static Vec3f kSpinAxis(0,1,0);

// Dont use this brah
TankShot::TankShot(){};

TankShot::TankShot(const PositionOrientation & tankPosition,
                   const GroundOrientaion & groundOrientation,
                   float barrelDirectionRads,
                   float barrelAngleRads,
                   float velocity,
                   const gl::GlslProgRef & shader,
                   const int parentContentID) :
mVelocity(velocity)
, mTankMat(ci::Matrix44f::identity())
, mGunMat(ci::Matrix44f::identity())
,mProgress(0)
,mHasExploded(false)
,mPointOfExplosion(0,0,0)
,mShotOrigin(0,0,0)
,mExplosionScale(1)
,mIsDead(false)
,mMaxProgress(0)
,mContentID(parentContentID)
,mGroundOrientation(groundOrientation)
,mTankOrientation(tankPosition)
{
    if (!ExplosionTexture)
    {
        ExplosionTexture = gl::Texture::create(loadImage(ci::app::loadResource("explosion.png")));
    }
    
    mTankMat = Matrix44f::identity();
    
    // Tank Mat
    // Adjust to tank orientation
    mTankMat.rotate(Vec3f(0, 1, 0), toRadians(tankPosition.directionDegrees));
    // Adjust the height
    mTankMat.translate(Vec3f(0, groundOrientation.height, 0));
    // X & Y angles
    mTankMat.rotate(Vec3f(1, 0, 0), groundOrientation.xAngleRads - (M_PI * 0.5));
    mTankMat.rotate(Vec3f(0, 0, 1), groundOrientation.zAngleRads - (M_PI * 0.5));
    
    // Gun Mat
    mGunMat = Matrix44f::identity();
    // Barrel Spin rotation
    mGunMat.rotate(kSpinAxis, barrelDirectionRads);
    // Raise to barrel height
    mGunMat.translate(Vec3f(0,kTankBarrelOffsetY,0));
    // Barrel shot angle
    mGunMat.rotate(kAngleAxis, barrelAngleRads);
    // Get the distance
    float x = cos(barrelAngleRads) * kTankBarrelLength;
    float y = sin(barrelAngleRads) * kTankBarrelLength;
    float dist = sqrt(x*x + y*y);
    // Barrel length (tip) (on Z axis)
    mGunMat.translate(kShotAxis * dist);

    generateLine(shader);
};

int TankShot::getContentID()
{
    return mContentID;
}

bool TankShot::isDead()
{
    return mIsDead;
}

const static float kLineProgressInterval = 0.5;

void TankShot::generateLine(const gl::GlslProgRef & shader)
{
    TriMesh::Format meshFormat = TriMesh::Format().positions();
    TriMesh lineMesh(meshFormat);

    Matrix44f tankGunTransformMat = mTankMat * mGunMat;
    Vec3f shotOrigin = tankGunTransformMat.transformPoint(Vec3f(0,0,0));
    
    Vec3f barrelBase = Vec3f(0,kTankBarrelOffsetY,0);
    barrelBase = mTankMat.transformPoint(barrelBase);

    Vec3f relativeOrigin = shotOrigin - barrelBase;
    float roY = relativeOrigin.y;
    float roX = Vec3f(relativeOrigin.x, 0, relativeOrigin.z).distance(Vec3f::zero());
    float shotTheta = atan2(roY, roX);
    
    bool didHitGround = false;
    float progress = 0;
    
    Vec3f worldOffset = mTankOrientation.position;

    while(!didHitGround)
    {
        float shotY = mVelocity*sin(shotTheta)*progress;
        float shotX = mVelocity*cos(shotTheta)*progress;
        // Converts the X to whatever the shot axis is
        Vec3f shotPos = Vec3f(0,shotY,0) + (Vec3f(shotX,shotX,shotX) * kShotAxis);
        
        shotPos += Vec3f(0,roY,roX);
        
        float relativeAngle = atan2(relativeOrigin.z, relativeOrigin.x);
        shotPos.rotate(Vec3f(0,1,0), -relativeAngle + (M_PI * 0.5));
        
        // NOTE: Added world offset here...
        Vec3f worldPos = barrelBase + shotPos + worldOffset;
        worldPos.y -= 0.5*kGravity*progress*progress;
        
        lineMesh.appendVertex(worldPos);
        
        // NOTE: We could check for the height at this x,z but it's probably overkill right now.
        if (worldPos.y <= mGroundOrientation.height)
        {
            mMaxProgress = progress;
            didHitGround = true;
            mPointOfExplosion = worldPos;
        }
        
        if (progress == 0)
        {
            mShotOrigin = worldPos;
        }
        
        progress += kLineProgressInterval;

    }
    
    mLineVao = gl::Vao::create();
    mLineVao->bind();
    
    mLineVbo = gl::Vbo::create(GL_ARRAY_BUFFER, 3 * lineMesh.getNumVertices() * sizeof(float),
                               lineMesh.getVertices<3>(), GL_STATIC_DRAW );
    mLineVbo->bind();
    
    GLint pos = shader->getAttribSemanticLocation( geom::Attrib::POSITION );
    gl::enableVertexAttribArray( pos );
    gl::vertexAttribPointer( pos, 3, GL_FLOAT, GL_FALSE, 0, 0);

    mLineVbo->unbind();
    mLineVao->unbind();
}

void TankShot::update(float amount)
{
    mProgress += amount;
    float progress = std::min<float>(mProgress, mMaxProgress);
    if (!mHasExploded)
    {
        if (progress == mMaxProgress)
        {
            ci::app::console() << "EXPLODE!!\n";
            mHasExploded = true;
            mExplosionScale = 1.5f;
        }
    }
    else
    {
        // Countdown to death
        mExplosionScale *= 0.95;
        if (mExplosionScale < 0.05)
        {
            mIsDead = true;
        }
    }
    
}

void TankShot::renderLine()
{
    mLineVao->bind();
    mLineVbo->bind();

    gl::pushMatrices();

    float progress = std::min<float>(mProgress, mMaxProgress);
    int numVerts = floor(progress / kLineProgressInterval);

    gl::setDefaultShaderVars();
    gl::drawArrays(GL_LINES, 0, numVerts);
    
    gl::popMatrices();

    mLineVbo->unbind();
    mLineVao->unbind();
}

void TankShot::renderMuzzleFlare(ci::CameraPersp & cam)
{
    const static float kMussleFlareProgress = 3.5f;
    if (mProgress < kMussleFlareProgress)
    {
        float amtFlare = 1.0 - (mProgress/kMussleFlareProgress);

        gl::pushMatrices();
        
        gl::translate(mShotOrigin);

        Vec3f right, up;
        cam.getBillboardVectors(&right, &up);
        ci::Matrix44<float> billboardMat(right, up, Vec3f::zero());
        gl::multModelView(billboardMat);
        
        gl::setDefaultShaderVars();
        Rectf blastRect(-64, -64, 64, 64);
        blastRect *= amtFlare;
        gl::draw(ExplosionTexture, blastRect);
        gl::popMatrices();
    }
}

void TankShot::renderExplosion(ci::CameraPersp & cam)
{
    if (mHasExploded)
    {
        gl::pushMatrices();

        gl::translate(mPointOfExplosion);

        Vec3f right, up;
        cam.getBillboardVectors(&right, &up);
        ci::Matrix44<float> billboardMat(right, up, Vec3f::zero());
        gl::multModelView(billboardMat);
        
        gl::setDefaultShaderVars();
        Rectf blastRect(-512, -512, 512, 512);
        blastRect *= std::min<float>(mExplosionScale, 1.0f);
        gl::draw(ExplosionTexture, blastRect);
        gl::popMatrices();
    }
}
