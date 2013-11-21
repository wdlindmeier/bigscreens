//
//  TankConvergenceContent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/31/13.
//
//

#include "TankConvergenceContent.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace ci::app;
using namespace bigscreens;

TankConvergenceContent::TankConvergenceContent() :
DumbTankContent()
, mOpponent(ActorContentProvider::getOpponent())
, mMSElapsedConvergence(0)
{
    mDumbTank = ActorContentProvider::getAngledDumbTank();
    mGroundScale = Vec3f(15000, 600, 15000);
    //mGroundScale = Vec3f(10000, 600, 10000);
};

void TankConvergenceContent::setMSElapsed(const long msElapsedConvergence)
{
    mMSElapsedConvergence = msElapsedConvergence;
}

CameraOrigin TankConvergenceContent::cameraForTankConvergence(int regionIndex,
                                                              int regionCount,
                                                              long msOffset,
                                                              const Vec2i & masterSize,
                                                              const Rectf & regionRect)
{
    // These are arbitrary numbers
    float camXOffsetInterval = 4000.0f / regionCount;
    float camYOffsetInterval = 500.0f / regionCount;
    float camZOffsetInterval = camXOffsetInterval;
    
    PositionOrientation tankOrient = TankConvergenceContent::positionForTankWithProgress(regionIndex, msOffset);
    Vec3f tankPos = tankOrient.position;
    
    CameraOrigin orig;
    
    // TODO: Make this more interesting
    orig.eye = Vec3f(tankPos.x + ((camXOffsetInterval * regionIndex) - 2000.0f), // 
                     tankPos.y + 1500 + (((camYOffsetInterval * regionIndex) - 250.0f)),
                     tankPos.z + ((camZOffsetInterval * regionIndex) - 2000.0f));
    
    // Look at the tank
    orig.target = Vec3f(tankPos.x, 100, tankPos.z);
    
    // A horizontal lens shift of 1 (-1) will shift the view right (left) by half the width of the viewport.
    // A vertical lens shift of 1 (-1) will shift the view up (down) by half the height of the viewport.
    Vec2f viewCenter(masterSize.x * 0.5, masterSize.y * 0.5);
    Vec2f regCenter = regionRect.getCenter();
    float horizShift = 1.0 - (regCenter.x / viewCenter.x);
    float vertShift = 1.0 - (regCenter.y / viewCenter.y);
    orig.camShift = Vec2f(horizShift, vertShift);
    
    return orig;
}

// TODO: Update opponent

PositionOrientation TankConvergenceContent::positionForTankWithProgress(const int tankNum, long msOffset)
{
    float scalarProgress = 1.0 - std::min<float>(1.0, (float)msOffset / (float)kMSFullConvergence);
    float weightedProgress = scalarProgress * scalarProgress;
    
    float scalarOffset = (float)tankNum / (float)kNumTanksConverging;
    float rads = M_PI * 2 * scalarOffset;
    
    const static float kMaxTankDist = 22000.0f;
    const static float kMinTankDist = 4000.0f;
    const static float kTankTravelDist = kMaxTankDist - kMinTankDist;
    
    // A simple distance based on sin that looks somewhat staggered
    float iDist = fabs(sin(tankNum)) * kTankTravelDist;
    
    float tankDist = kMinTankDist + (iDist * weightedProgress);
    float x = cos(rads) * tankDist;
    float y = 0;
    float z = sin(rads) * tankDist;
    PositionOrientation orientation;
    orientation.position = Vec3f(x,y,z);

    // This is always a circle
    orientation.directionDegrees = 270.0 - ci::toDegrees(rads);
    
    return orientation;
}

void TankConvergenceContent::render(const ci::Vec2i & screenOffset,
                                    const ci::Rectf & contentRect)
{
    render(screenOffset, contentRect, 1.0f);
}

void TankConvergenceContent::fireTankGun(const int tankIndex)
{
    // This kind of sucks... but it works
    int fromIdx = 0;
    int upToIdx = kNumTanksConverging;
    if (tankIndex != -1)
    {
        fromIdx = tankIndex;
        upToIdx = tankIndex + 1;
    }
        
    for (int i = fromIdx; i < upToIdx; ++i)
    {
        PositionOrientation tankOrient = positionForTankWithProgress(i, mMSElapsedConvergence);
        setTankPosition(tankOrient.position, toRadians(tankOrient.directionDegrees));
        TankContent::fireTankGun(mDumbTank);
    }
}

void TankConvergenceContent::renderOpponent(const float alpha)
{
    // Draw the opponent
    gl::pushMatrices();
    gl::setMatrices(mCam);
    gl::scale(Vec3f(1000, 1000, 1000));
    
    // Arbitrary
    Vec3f lightPos(sin(mNumFramesRendered * 0.1),
                   cos(mNumFramesRendered * 0.06666),
                   cos(mNumFramesRendered * 0.03333));
    
    float zDepth = mCam.getEyePoint().length();
    // TODO: This should take an alpha so the opponent fades in w/ the rest
    mOpponent->draw(zDepth, lightPos);
    
    gl::popMatrices();
}

void TankConvergenceContent::render(const ci::Vec2i & screenOffset,
                                    const ci::Rectf & contentRect,
                                    const float alpha)
{
    
    mRenderAlpha = ci::math<float>::clamp(alpha, 0, 1.0);
    mScreenAlpha = mRenderAlpha*mRenderAlpha*mRenderAlpha;
    
    // Draw the ground and tanks
    gl::enableAlphaBlending();

    gl::disableDepthRead();
    gl::disableDepthWrite();
    
    std::pair<Vec2i,Vec2i> viewport = gl::getViewport();
    drawScreen(screenOffset, contentRect);
    gl::viewport(viewport.first, viewport.second);

    renderOpponent(alpha);
    
    // Draw the ground and tanks
    gl::enableAlphaBlending();
    
    // NOTE: Having issues w/ depth and blending during convergence
    const bool kUseGroundDepthTest = false;
    if (kUseGroundDepthTest)
    {
        gl::enableDepthWrite();
        gl::enableDepthRead();
    }
    drawGround();
    
    gl::enableAlphaBlending();

    drawTank();
    
    DumbTankContent::drawTankShots();
    
    if (kUseGroundDepthTest)
    {
        gl::disableDepthWrite();
        gl::disableDepthRead();
    }
    
}

// NOTE: This draws a collection of tanks
void TankConvergenceContent::drawTank()
{
    // Make the tanks ease into position

    mTankShader->bind();
    
    gl::VaoRef vao = mDumbTank->getVao();
    vao->bind();

    gl::VboRef vbo = mDumbTank->getElementVbo();
    vbo->bind();

    gl::pushMatrices();
    gl::setMatrices( mCam );

    mTankShader->uniform("uColor", ColorAf(1, 1, 1, 0.5f * mRenderAlpha));
    
    for (int i = 0; i < kNumTanksConverging; ++i)
    {
        PositionOrientation tankOrient = positionForTankWithProgress(i, mMSElapsedConvergence);
        setTankPosition(tankOrient.position, toRadians(tankOrient.directionDegrees));
        TankContent::drawTank();
    }

    gl::popMatrices();

    vbo->unbind();
    vao->unbind();
    
    mTankShader->unbind();
}

void TankConvergenceContent::renderPositionedTank()
{
    // Called from TankContent::DrawTank after positioning
    gl::setDefaultShaderVars();
    gl::drawElements(GL_LINES,
                     mDumbTank->getMesh()->getNumIndices(),
                     GL_UNSIGNED_INT, 0 );
}

void TankConvergenceContent::drawScreen(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect)
{
    gl::pushMatrices();
    
    // Temporarily resetting the viewport.
    // It's set back in draw().
    gl::viewport(contentRect.x1 - screenOffset.x,
                 getWindowHeight() - contentRect.y2 - screenOffset.y,
                 contentRect.getWidth(),
                 contentRect.getHeight());
    
    gl::setMatricesWindow(contentRect.getWidth(),
                          contentRect.getHeight());
    
    gl::scale(contentRect.getWidth(), contentRect.getHeight());
    
    mTextureShader->bind();
    mScreenTexture->bind();
    
    gl::setDefaultShaderVars();

    // No offset
    mTextureShader->uniform("uTexCoordOffset", Vec2f(0,0));
    mTextureShader->uniform("uColor", ColorAf(1, 1, 1, mScreenAlpha));
    
    mScreenVao->bind();
    mScreenVbo->bind();
    
    gl::drawArrays( GL_TRIANGLE_STRIP, 0, 4 );
    
    mScreenVao->unbind();
    mScreenVbo->unbind();
    
    mScreenTexture->unbind();
    mTextureShader->unbind();
    
    gl::popMatrices();
}

void TankConvergenceContent::updateGroundCoordsForTank()
{
    // Do nothing
}

void TankConvergenceContent::update(std::function<void (ci::CameraPersp & cam, DumbTankRef& tank)> update_func)
{
    // First update the opponent
    
    // Sample: This is the same as the light position
    Vec3f smokeDirection(sin(mNumFramesRendered * 0.1),
                         cos(mNumFramesRendered * 0.06666),
                         cos(mNumFramesRendered * 0.03333));

    // QUESTION: What is "percentage"?
    float percentage = sin(mNumFramesRendered * 0.1);
    mOpponent->update(percentage, smokeDirection);
    
    DumbTankContent::update(update_func);
}
