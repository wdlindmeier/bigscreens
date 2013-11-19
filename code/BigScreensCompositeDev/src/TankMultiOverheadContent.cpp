//
//  TankMultiOverheadContent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/18/13.
//
//

#include "TankMultiOverheadContent.h"

using namespace ci;
using namespace ci::app;
using namespace bigscreens;

PositionOrientation TankMultiOverheadContent::positionForTankWithProgress(const int tankNum,
                                                                          const long numFramesProgress)
{
    const static long kFramesFullCreep = 1000;

    // Let it run over
    float bend = (1 + fabs(sin(tankNum)));
    float scalarProgress = ((float)numFramesProgress / (float)kFramesFullCreep) * bend;

    const static float kMinTankDist = 4500.0f;
    const static float kMaxTankDist = -4000.0f;
    const static float kTankTravelDist = kMaxTankDist - kMinTankDist;
    
    // A simple distance based on sin that looks somewhat staggered
    float startDist = kMinTankDist + (fabs(sin(tankNum)) * (kTankTravelDist * -0.25));
    
    float tankDist = startDist + (kTankTravelDist * scalarProgress);

    const static float kXSpan = 80000.0f;
    const static float kXInterval = kXSpan / kNumTanksStreaming;
    float x = (kXInterval * tankNum) - (kXSpan * 0.5f) + (sin(tankNum) * (kXInterval * 0.5)) + (sin((numFramesProgress * tankNum) * 0.005) * 10);
    float y = 0;
    float z = tankDist;
    
    PositionOrientation orientation;
    orientation.position = Vec3f(x,y,z);
    orientation.directionDegrees = 180.0f;
    
    return orientation;
}

// NOTE: This draws a collection of tanks
void TankMultiOverheadContent::drawTank()
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
    
    for (int i = 0; i < kNumTanksStreaming; ++i)
    {
        PositionOrientation tankOrient = positionForTankWithProgress(i, mNumFramesRendered);
        setTankPosition(tankOrient.position, toRadians(tankOrient.directionDegrees));
        TankContent::drawTank();
    }
    
    gl::popMatrices();
    
    vbo->unbind();
    vao->unbind();
    
    mTankShader->unbind();
}

void TankMultiOverheadContent::renderPositionedTank()
{
    // Called from TankContent::DrawTank after positioning
    gl::setDefaultShaderVars();
    gl::drawElements(GL_LINES,
                     mDumbTank->getMesh()->getNumIndices(),
                     GL_UNSIGNED_INT, 0 );
}