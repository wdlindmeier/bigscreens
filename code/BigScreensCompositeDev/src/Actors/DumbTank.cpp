//
//  DumbTank.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/10/13.
//
//

#include "DumbTank.h"

using namespace bigscreens;
using namespace ci;
using namespace ci::app;
using namespace std;

void DumbTank::load()
{
    loadShader();
    loadModels();
}

void DumbTank::loadShader()
{
    gl::GlslProg::Format mFormat;
    mFormat.vertex( loadResource( "basic.vert" ) )
    .fragment( loadResource( "basic.frag" ) );
    mTankShader = gl::GlslProg::create( mFormat );
    mTankShader->bind();
}

void DumbTank::loadModels()
{
    mTankModel.load("tank.obj", mTankShader);
}

ObjModel & DumbTank::getModel()
{
    return mTankModel;
}

void DumbTank::render(ci::CameraPersp & cam, const float alpha)
{
    gl::enableAdditiveBlending();
    
    mTankShader->bind();
    
    gl::setDefaultShaderVars();
    
    // NOTE: There are so many lines, we throttle the alpha to 0.25 max
    mTankShader->uniform("uColor", ColorAf( 1, 1, 1, alpha*0.25f ));
    
    mTankModel.render();

    mTankShader->unbind();
}
