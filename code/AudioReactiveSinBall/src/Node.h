//
//  Node.h
//  AudioReactiveSinBall
//
//  Created by William Lindmeier on 9/11/13.
//
//

#pragma once

#include "cinder/Cinder.h"

class Node
{
    
public:
    
    Node(const float scalarNode) : mScalarNode(scalarNode), mPrevBandValue(0) {};
    
    void update(const float fftBandValue,
                float minSize,
                float maxSize,
                float attackWeight,
                float gainWeight);
    
    void draw(const ci::Vec3f & position, const ci::Vec3f & axisProgress);
    
private:
    
    float mScalarNode;
    float mAttackSize;
    float mGainSize;
    float mSize;
    float mPrevBandValue;
    
};