//
//  TankBlinkingContent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/31/13.
//
//

#include "TankBlinkingContent.h"
#include "cinder/Rand.h"

using namespace bigscreens;
using namespace ci;
using namespace ci::app;

void TankBlinkingContent::drawTank()
{
    gl::clear( Color( 0, 0, 0 ) );
    
    gl::enableAdditiveBlending();
    
    mTankShader->bind();
    gl::pushMatrices();
    gl::setMatrices( mCam );
    
    gl::setDefaultShaderVars();
    
    mTankVao->bind();
    mTankElementVbo->bind();
    
    int numIndices = mTankMesh->getNumIndices();
    
    mTankShader->uniform("uColor", ColorAf(1,1,1,0.05f));
    gl::drawElements(GL_LINES,
                     numIndices,
                     GL_UNSIGNED_INT,
                     0);
    
    mTankShader->uniform("uColor", ColorAf(1,1,1,1.0f));
    float scalarCompleteness = sin(getElapsedFrames() * 0.01);
    int frameCount = scalarCompleteness * numIndices;
    frameCount = frameCount - (frameCount % 2);
    int count = std::min<int>(frameCount, numIndices);
    int offset = Rand::randInt(numIndices - count);
    offset = offset - (offset % 2);
    gl::drawElements(GL_LINES,
                     count,
                     GL_UNSIGNED_INT,
                     (void*)(offset * sizeof(GLuint)));
    
    mTankElementVbo->unbind();
    mTankVao->unbind();
    
    mTankShader->unbind();
    gl::popMatrices();
}