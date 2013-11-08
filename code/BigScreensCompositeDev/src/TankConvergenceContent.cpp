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


TankConvergenceContent::TankConvergenceContent() : TankContent()
{
    mGroundContent = GroundContent(20000.0);
};

TankOrientation TankConvergenceContent::positionForTankWithProgress(const int tankNum, long frameProgress)
{
    float scalarProgress = 1.0 - std::min<float>(1.0, (float)frameProgress / (float)kNumFramesTanksConverge);
    float weightedProgress = scalarProgress * scalarProgress;
    
    float scalarOffset = (float)tankNum / (float)kNumTanksConverging;
    float rads = M_PI * 2 * scalarOffset;
    
    // A simple distance based on sin that looks somewhat staggered
    float iDist = fabs(sin(tankNum)) * 5000;
    
    float tankDist = 5000 + (iDist * weightedProgress);
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
    
    mScreenAlpha = 1.0f - (((float)mNumFramesRendered / (float)kNumFramesTanksConverge) * 2.0f);
    
    // NOTE: Don't clear
    
    std::pair<Vec2i,Vec2i> viewport = gl::getViewport();
    drawScreen(contentRect);
    gl::viewport(viewport.first, viewport.second);
    
    drawGround();
    
    drawTank();
}

// NOTE: This draws a collection of tanks
void TankConvergenceContent::drawTank()
{
    // Make the tanks ease into position
    mTankShader->bind();
    mTankVao->bind();
    mTankElementVbo->bind();

    gl::pushMatrices();
    gl::setMatrices( mCam );
    
    mTankShader->uniform("uColor", ColorAf(1,1,1,mRenderAlpha));
    
    for (int i = 0; i < kNumTanksConverging; ++i)
    {
        TankOrientation tankOrient = positionForTankWithProgress(i, mNumFramesRendered);
        drawSingleTankAtPosition(tankOrient.position,
                                 tankOrient.directionDegrees);
    }
    
    gl::popMatrices();
    mTankElementVbo->unbind();
    mTankVao->unbind();
    mTankShader->unbind();
}

void TankConvergenceContent::drawScreen(const ci::Rectf & contentRect)
{
    gl::pushMatrices();
    
    // Temporarily resetting the viewport.
    // It's set back in draw().
    gl::viewport(contentRect.x1,
                 getWindowHeight() - contentRect.y2,
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
    mTankShader->bind();

    gl::enableAlphaBlending();
    
    mTankShader->uniform("uColor", ColorAf(0.75,0.75,0.75, mRenderAlpha));
    
    // Get the current plot
    float groundScale = mGroundContent.getScale();
    Vec3f groundOffset((-0.5f * groundScale),
                       0,
                       (-0.5f * groundScale));
    
    mGroundContent.render(GL_LINE_STRIP, groundOffset);

    mTankShader->unbind();
    
    gl::popMatrices();
}


void TankConvergenceContent::drawSingleTankAtPosition(const Vec3f & position, const float rotationDegrees)
{
    gl::pushMatrices();
    gl::translate(position);
    gl::rotate(rotationDegrees, 0, 1, 0);
    
    gl::setDefaultShaderVars();

    gl::drawElements( GL_LINES, mTankMesh->getNumIndices(), GL_UNSIGNED_INT, 0 );
    gl::popMatrices();
}