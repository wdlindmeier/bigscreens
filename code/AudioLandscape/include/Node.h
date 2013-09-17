//
//  Node.h
//  NodeMesh
//
//  Created by William Lindmeier on 9/15/13.
//
//

#pragma once

#include "cinder/Cinder.h"
#include <vector>
#include "cinder/Color.h"
#include "cinder/Matrix.h"

class Playhead;

class Node
{
  
public:
    
    Node() :
    mColor(0.2f, 0.6f, 0.8f, 1.0f),
    mRadius(2.0f),
    mPosition(ci::Vec3f::zero()),
    mScale(1.0f),
    mDecay(0.95f),
    mMaxScale(15.0f){};
    
    ~Node(){};
    
    void        draw();
    void        update();
    void        amplify(const float audioMagnitude);
    
    void        setPosition(const ci::Vec3f & position);
    ci::Vec3f   getPosition();

    void        setColor(const ci::ColorA & color);
    ci::ColorA  getColor();

    void        setRadius(const float radius);
    float       getRadius();

    void        setDecay(const float decay);
    float       getDecay();

    void        setMaxScale(const float scale);
    float       getMaxScale();

    ci::Matrix44f getModelMatrix();

protected:
    
    ci::Vec3f   mPosition;
    float       mDecay;
    float       mRadius;
    float       mScale;
    ci::ColorA  mColor;
    float       mMaxScale;
    
};