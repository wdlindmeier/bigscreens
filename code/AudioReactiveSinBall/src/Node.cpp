//
//  Node.cpp
//  AudioReactiveSinBall
//
//  Created by William Lindmeier on 9/11/13.
//
//

#include "Node.h"

#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"

using namespace ci;

void Node::update(const float fftBandValue,
                  float minSize,
                  float maxSize,
                  float attackWeight,
                  float gainWeight)
{
    // Lets say the amount of averaging depends on the band delta.
    // This should be 0-1
    float bandDelta = fabs(fftBandValue - mPrevBandValue);
    mPrevBandValue = fftBandValue; //ci::math<float>::clamp(fftBandValue, 0, 1.0);
    
    float deltaSize = minSize + (bandDelta * (maxSize - minSize));
    mAttackSize = (deltaSize * attackWeight) + (mAttackSize * (1.0-attackWeight));

    float gainSize = minSize + (fftBandValue * (maxSize - minSize));
    mGainSize = (gainSize * gainWeight) + (mGainSize * (1.0-gainWeight));
    
    mSize = ((mGainSize * gainWeight) + (mAttackSize * attackWeight)) / (gainWeight + attackWeight);
}

void Node::draw(const Vec3f & position, const Vec3f & axisProgress)
{
    // red == 0.5-1.0
    //float r = std::max<float>(0.0f, mScalarNode-0.5f) / 0.5f;
    float r = 1.0f - mScalarNode;
    
    // green == 0.25-0.75
    //float g = std::max<float>(0.0f, 1.0f - (fabs(mScalarNode - 0.5f) * 4.0f));
    float g = std::max<float>(0.0f, 1.0f - (fabs(mScalarNode - 0.5f) * 4.0f));
    
    // blue == 0.0 - 0.5
    //float b = 1.0f - std::min<float>(1.0f, mScalarNode/0.5f);
    float b = mScalarNode; //std::min<float>(1.0f, mScalarNode/0.5f);
    
    gl::color(r, g, b);
    
    gl::drawSphere(position, mSize, 32);
}
