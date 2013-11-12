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
    ObjModel *tankModel = new ObjModel();
    tankModel->load("tank.obj", mTankShader);
    mTankModel = ObjModelRef(tankModel);
}

ObjModelRef & DumbTank::getModel()
{
    return mTankModel;
}

void DumbTank::render(ci::CameraPersp & cam, const float alpha)
{
    gl::enableAdditiveBlending();
    
    mTankShader->bind();
    
    gl::setDefaultShaderVars();
    
    mTankShader->uniform("uColor", ColorAf( 1, 1, 1, alpha ));
    
    mTankModel->render();

    mTankShader->unbind();
}
