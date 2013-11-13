//
//  Trajectory.cpp
//  AdvancedTank
//
//  Created by William Lindmeier on 11/3/13.
//
//

#include "TankShot.h"
#include "cinder/TriMesh.h"

using namespace ci;
using namespace bigscreens;

// TODO: Make a shared asset manager
static gl::TextureRef ExplosionTexture;

// Dont use this brah
TankShot::TankShot(){};

TankShot::TankShot(float angleRads,
                   float yRotationRads,
                   float velocity,
                   const ci::Vec3f & initialPosition,
                   const gl::GlslProgRef & shader,
                   const int parentContentID) :
mVelocity(velocity)
,mYRotationRads(yRotationRads)
,mThetaRads(angleRads)
,mProgress(0)
,mInitialPosition(initialPosition)
,mHasExploded(false)
,mCurrentPosition(initialPosition)
,mPointOfExplosion(0,0,0)
,mCurrentOffset(0,0)
,mExplosionScale(1)
,mIsDead(false)
,mMaxProgress(0)
,mContentID(parentContentID)
{
    mShotQuat = ci::Quatf(Vec3f(0,1,0), mYRotationRads);
    if (!ExplosionTexture)
    {
        ExplosionTexture = gl::Texture::create(loadImage(ci::app::loadResource("explosion.png")));
    }
    generateLine(shader);
};

float TankShot::getVelocity()
{
    return mVelocity;
}

float TankShot::getThetaRads()
{
    return mThetaRads;
}

float TankShot::getProgress()
{
    return mProgress;
}

int TankShot::getContentID()
{
    return mContentID;
}

bool TankShot::isDead()
{
    return mIsDead;
}

ci::Vec2f TankShot::progressAt(float amount)
{
//    float y = mVelocity*sin(mThetaDegrees*pi/180.0)*amount - 0.5*kGravity*amount*amount;  // altitude
//    float x = mVelocity*cos(mThetaDegrees*M_PI/180.0)*amount;  // downrange
    float y = mVelocity*sin(mThetaRads)*amount - 0.5*kGravity*amount*amount;  // altitude
    float x = mVelocity*cos(mThetaRads)*amount;  // downrange
    return Vec2f(x,y);
}

const static float kLineProgressInterval = 0.5;

void TankShot::generateLine(const gl::GlslProgRef & shader)
{
    TriMesh::Format meshFormat = TriMesh::Format().positions();
    TriMesh lineMesh(meshFormat);
    
    bool didHitGround = false;
    float progress = 0;

    Quatf shotQuat = ci::Quatf(Vec3f(0,1,0), mYRotationRads);

    while(!didHitGround)
    {
        progress += kLineProgressInterval;
        Vec2f segOffset = progressAt(progress);

        Vec3f segPosition = mInitialPosition + Vec3f(0, segOffset.y, segOffset.x);

        // Attempting to rotate around the Y axis
        segPosition = shotQuat * segPosition;
        
        lineMesh.appendVertex(segPosition);
        if (segPosition.y <= 0)
        {
            mMaxProgress = progress;
            didHitGround = true;
        }
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
    mCurrentOffset = progressAt(mProgress);
    
    Vec3f missleCenter(0, mCurrentOffset.y, mCurrentOffset.x);
    
    if (!mHasExploded)
    {
        mCurrentPosition = mInitialPosition + missleCenter;
        mCurrentPosition = mShotQuat * mCurrentPosition;
        
        if (mCurrentOffset.x <= 0 || mCurrentOffset.y < (mInitialPosition.y * -1))
        {
            mHasExploded = true;
            // mIsDead = true;
            mPointOfExplosion = mCurrentPosition;
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

    float progress = std::min<float>(mProgress, mMaxProgress);
    int numVerts = floor(progress / kLineProgressInterval);

    gl::setDefaultShaderVars();
    gl::drawArrays(GL_LINES, 0, numVerts);
    
    mLineVbo->unbind();
    mLineVao->unbind();
}

void TankShot::render()
{
    if (!mHasExploded)
    {
        gl::setDefaultShaderVars();
        gl::drawCube(mCurrentPosition, Vec3f(50,50,50));
    }
}

void TankShot::renderMuzzleFlare(ci::CameraPersp & cam)
{
    const static float kMussleFlareProgress = 3.5f;
    if (mProgress < kMussleFlareProgress)
    {
        float amtFlare = 1.0 - (mProgress/kMussleFlareProgress);
        Vec3f muzzlePosition = mInitialPosition * mShotQuat;
        
        gl::pushMatrices();
        gl::translate(muzzlePosition);
        
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
