//
//  DumbTankContent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/14/13.
//
//

#include "DumbTankContent.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace ci::app;
using namespace bigscreens;

DumbTankContent::DumbTankContent() :
TankContent()
,mDumbTank(ContentProviderNew::ActorContent::getDumbTank())
{
};

void DumbTankContent::loadShaders()
{
    console() << "LOAD DUMB SHADERS\n";
    TankContent::loadShaders();
    gl::GlslProg::Format shaderFormat;
    shaderFormat.vertex( ci::app::loadResource( "basic.vert" ) )
    .fragment( ci::app::loadResource( "basic.frag" ) );
    mTankShader = ci::gl::GlslProg::create( shaderFormat );
    mTankShader->uniform("uColor", Color::white());
}

void DumbTankContent::update(std::function<void (ci::CameraPersp & cam, DumbTankRef& tank)> update_func)
{
    update_func(mCam, mDumbTank);
}

void DumbTankContent::drawMinion()
{
    // Not for now
    return;
}

void DumbTankContent::drawTankShots()
{
    // Not for now
}

void DumbTankContent::renderPositionedTank()
{
    gl::enableAdditiveBlending();
    
    mTankShader->bind();
    
    gl::VaoRef vao = mDumbTank->getVao();
    vao->bind();
    
    gl::VboRef vbo = mDumbTank->getElementVbo();
    vbo->bind();

    mTankShader->uniform("uColor", ColorAf(1,1,1,0.5));
    
    gl::setDefaultShaderVars();
    gl::drawElements(GL_LINES,
                     mDumbTank->getMesh()->getNumIndices(),
                     GL_UNSIGNED_INT, 0 );

    vbo->unbind();
    vao->unbind();
    
    mTankShader->unbind();
}