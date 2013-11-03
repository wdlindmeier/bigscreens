//
//  AdvancedTank.cpp
//  AdvancedTank
//
//  Created by William Lindmeier on 11/3/13.
//
//

#include "AdvancedTank.h"

using namespace ci;
using namespace ci::app;
using namespace bigscreens;

void AdvancedTank::load()
{
    loadShader();
    loadModels();
}

void AdvancedTank::loadShader()
{
    gl::GlslProg::Format mFormat;
    mFormat.vertex( loadResource( "basic.vert" ) )
    .fragment( loadResource( "basic.frag" ) );
    mTankShader = gl::GlslProg::create( mFormat );
    mTankShader->bind();
}

void AdvancedTank::loadModels()
{
    mBodyModel.load("tank_body.obj", mTankShader);
    mHeadModel.load("tank_head.obj", mTankShader);
    mBarrelModel.load("tank_barrel_centered.obj", mTankShader);
    mGearWheelModel.load("tank_gear_wheel_centered.obj", mTankShader);
    mWheelModel.load("tank_wheel_centered.obj", mTankShader);
}

void AdvancedTank::update()
{
    mProgress += 1;
}

void AdvancedTank::render(const ci::Vec2f & screenOffset)
{
    gl::enableAdditiveBlending();
    
    mTankShader->bind();

    gl::setDefaultShaderVars();
    
    mTankShader->uniform("uColor", ColorAf(1,1,1,1));
    
    float wheelRotation = fmod(mProgress * 1, 360.0f);
    float gearRotation = wheelRotation * 1.5f; // gear is smaller

    float barrelAngle = ((1.0 + sin(mProgress * 0.025)) * 0.5) * -35.0f;
    
    mBodyModel.render();
    mHeadModel.render();
    
    // Barrel
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 170.0f, 0.0f));
    gl::rotate(barrelAngle, 1, 0, 0);
    mBarrelModel.render();
    gl::popMatrices();
    
    // Gear Wheel 1
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 79.86f, 222.193f));
    gl::rotate(gearRotation, 1, 0, 0);
    mGearWheelModel.render();
    gl::popMatrices();

    // Gear Wheel 2
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 79.86f, -339.32f));
    gl::rotate(gearRotation, 1, 0, 0);
    mGearWheelModel.render();
    gl::popMatrices();
    
    // Wheel 1
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, 151.78f));
    gl::rotate(wheelRotation, 1, 0, 0);
    mWheelModel.render();
    gl::popMatrices();

    // Wheel 2
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, 68.739f));
    gl::rotate(wheelRotation, 1, 0, 0);
    mWheelModel.render();
    gl::popMatrices();

    // Wheel 3
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, -14.03f));
    gl::rotate(wheelRotation, 1, 0, 0);
    mWheelModel.render();
    gl::popMatrices();

    // Wheel 4
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, -97.15f));
    gl::rotate(wheelRotation, 1, 0, 0);
    mWheelModel.render();
    gl::popMatrices();
    
    // Wheel 5
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, -180.04f));
    gl::rotate(wheelRotation, 1, 0, 0);
    mWheelModel.render();
    gl::popMatrices();
    
    // Wheel 6
    gl::pushMatrices();
    gl::translate(Vec3f(0.0f, 42.3f, -262.72f));
    gl::rotate(wheelRotation, 1, 0, 0);
    mWheelModel.render();
    gl::popMatrices();

    mTankShader->unbind();
}
