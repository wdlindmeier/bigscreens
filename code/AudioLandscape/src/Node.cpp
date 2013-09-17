//
//  Node.cpp
//  NodeMesh
//
//  Created by William Lindmeier on 9/15/13.
//
//

#include "Node.h"
#include "cinder/gl/gl.h"

using namespace ci;
using std::vector;

void    Node::draw()
{
    gl::drawSolidRect(Rectf(-1.0f, -1.0f, 1.0f, 1.0f));
    //gl::drawSolidCircle(Vec2f::zero(), 1.0f, 32);
}

void    Node::amplify(const float audioMagnitude)
{
    //mScale += audioMagnitude;
    mPosition.y -= (audioMagnitude * mMaxScale);
}

void    Node::update()
{
    //mScale = ci::math<float>::clamp(mScale * mDecay, 1.0f, mMaxScale);
    mPosition.y = mPosition.y * mDecay;
}

Matrix44f Node::getModelMatrix()
{
    Matrix44f matPosition = Matrix44f::createTranslation(mPosition);

    Vec3f vecScale = Vec3f(mRadius, mRadius, 1.0);
    vecScale *= mScale;
    Matrix44f matScale = Matrix44f::createScale(vecScale);

    return matPosition * matScale;
}

void    Node::setPosition(const Vec3f & position)
{
    mPosition = position;
}

Vec3f   Node::getPosition()
{
    return mPosition;
}

void    Node::setRadius(const float radius)
{
    mRadius = radius;
}

float   Node::getRadius()
{
    return mRadius;
}

void    Node::setMaxScale(const float scale)
{
//    assert(scale > 1.0f);
    mMaxScale = scale;
}

float   Node::getMaxScale()
{
    return mMaxScale;
}

void    Node::setColor(const ColorA & color)
{
    mColor = color;
}

ColorA  Node::getColor()
{
    return mColor;
}

void    Node::setDecay(const float decay)
{
    mDecay = decay;
}

float   Node::getDecay()
{
    return mDecay;
}

