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
TankContent()
, mMSElapsedConvergence(0)
, mDumbTank(ContentProviderNew::ActorContent::getDumbTank())
{
    mGroundContent = GroundContent(20000.0);
};

void TankConvergenceContent::setMSElapsed(const long msElapsedConvergence)
{
    mMSElapsedConvergence = msElapsedConvergence;
}

void TankConvergenceContent::loadShaders()
{
    TankContent::loadShaders();
    
    gl::GlslProg::Format groundShaderFormat;
    groundShaderFormat.vertex( ci::app::loadResource( "basic.vert" ) )
    .fragment( ci::app::loadResource( "basic.frag" ) );
    mGroundShader = ci::gl::GlslProg::create( groundShaderFormat );
    mGroundShader->uniform("uColor", Color::white());
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
    
    TankOrientation tankOrient = TankConvergenceContent::positionForTankWithProgress(regionIndex, msOffset);
    Vec3f tankPos = tankOrient.position;
    
    CameraOrigin orig;
    
    // TODO: Make this more interesting
    orig.eye = Vec3f(tankPos.x + ((camXOffsetInterval * regionIndex) - 2000.0f), // 
                     tankPos.y + 500 + (((camYOffsetInterval * regionIndex) - 250.0f)),
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

TankOrientation TankConvergenceContent::positionForTankWithProgress(const int tankNum, long msOffset)
{
    float scalarProgress = 1.0 - std::min<float>(1.0, (float)msOffset / (float)kMSFullConvergence);
    float weightedProgress = scalarProgress * scalarProgress;
    
    float scalarOffset = (float)tankNum / (float)kNumTanksConverging;
    float rads = M_PI * 2 * scalarOffset;
    
    const static float kMaxTankDist = 22000.0f;
    const static float kMinTankDist = 3000.0f;
    const static float kTankTravelDist = kMaxTankDist - kMinTankDist;
    
    // A simple distance based on sin that looks somewhat staggered
    float iDist = fabs(sin(tankNum)) * kTankTravelDist;
    
    float tankDist = kMinTankDist + (iDist * weightedProgress);
    float x = cos(rads) * tankDist;
    float y = 0;
    float z = sin(rads) * tankDist;
    TankOrientation orientation;
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

void TankConvergenceContent::render(const ci::Vec2i & screenOffset,
                                    const ci::Rectf & contentRect,
                                    const float alpha)
{
    mRenderAlpha = alpha;

    mScreenAlpha = mRenderAlpha*mRenderAlpha*mRenderAlpha;
    
    // NOTE: Don't clear
    
    std::pair<Vec2i,Vec2i> viewport = gl::getViewport();
    drawScreen(screenOffset, contentRect);
    gl::viewport(viewport.first, viewport.second);
    
    drawGround();
    
    drawTank();
}

void TankConvergenceContent::update(std::function<void (ci::CameraPersp & cam, DumbTankRef& tank)> update_func)
{
    update_func(mCam, mDumbTank);
}

// NOTE: This draws a collection of tanks
void TankConvergenceContent::drawTank()
{
    gl::pushMatrices();
    gl::setMatrices( mCam );
    
    // NOTE: Using the dumb tank because there's a lot going on in this scene and
    // we need to simplify it.
    
    // Make the tanks ease into position
    mGroundShader->bind();
    
    gl::VaoRef & vao = mDumbTank->getModel()->getVao();
    vao->bind();

    gl::VboRef & vbo = mDumbTank->getModel()->getElementVbo();
    vbo->bind();

    gl::pushMatrices();
    gl::setMatrices( mCam );
 
    mGroundShader->uniform("uColor", ColorAf(1,1,1,mRenderAlpha));
    
    for (int i = 0; i < kNumTanksConverging; ++i)
    {
        TankOrientation tankOrient = positionForTankWithProgress(i, mMSElapsedConvergence);
        drawSingleTankAtPosition(tankOrient.position,
                                 tankOrient.directionDegrees);
    }

    gl::popMatrices();
    vbo->unbind();
    vao->unbind();
    mGroundShader->unbind();
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

void TankConvergenceContent::drawGround()
{
    if (!mIsGroundVisible) return;
    
    gl::pushMatrices();
    
    gl::setMatrices( mCam );
    
    // NOTE: Not using the ground shader since it's a texture shader
    mGroundShader->bind();

    gl::enableAlphaBlending();
    
    mGroundShader->uniform("uColor", ColorAf(0.75,0.75,0.75, mRenderAlpha));
    
    // Get the current plot
    float groundScale = mGroundContent.getScale();
    Vec3f groundOffset((-0.5f * groundScale),
                       0,
                       (-0.5f * groundScale));
    
    mGroundContent.render(GL_LINE_STRIP, groundOffset);

    mGroundShader->unbind();
    
    gl::popMatrices();
}

void TankConvergenceContent::drawSingleTankAtPosition(const Vec3f & position, const float rotationDegrees)
{
    gl::pushMatrices();
    gl::translate(position);
    gl::rotate(rotationDegrees, 0, 1, 0);
    
    // mTank->render(mCam, mRenderAlpha);

    gl::setDefaultShaderVars();
    gl::drawElements(GL_LINES,
                     mDumbTank->getModel()->getMesh()->getNumIndices(),
                     //mTankMesh->getNumIndices(),
                     GL_UNSIGNED_INT, 0 );
    
    gl::popMatrices();
}