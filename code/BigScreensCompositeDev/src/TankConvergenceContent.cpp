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
    // NOTE: THis is always a circle
    orientation.directionDegrees = 270.0 - ci::toDegrees(rads);
    return orientation;
}

void TankConvergenceContent::render(const ci::Vec2i & screenOffset)
{
    mRenderAlpha = 1.0f;
    TankContent::render(screenOffset);
}

void TankConvergenceContent::render(const ci::Vec2i & screenOffset, const float alpha)
{
    mRenderAlpha = alpha;
    TankContent::render(screenOffset);
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

void TankConvergenceContent::drawSingleTankAtPosition(const Vec3f & position, const float rotationDegrees)
{
    gl::pushMatrices();
    gl::translate(position);
    gl::rotate(rotationDegrees, 0, 1, 0);
    
    gl::setDefaultShaderVars();

    gl::drawElements( GL_LINES, mTankMesh->getNumIndices(), GL_UNSIGNED_INT, 0 );
    gl::popMatrices();
}