//
//  SampleRenderable.cpp
//  ConvergenceTest
//
//  Created by William Lindmeier on 11/6/13.
//
//

#include "SampleRenderable.h"

using namespace ci;
using namespace ci::gl;
using namespace ci::app;
using namespace bigscreens;

SampleRenderable::SampleRenderable() :
mGroundContent(10000.0)
{
    mCam.setPerspective( 45.0f, getWindowAspectRatio(), .01, 40000 );
};

void SampleRenderable::load()
{
    // Load shader
    gl::GlslProg::Format mFormat;
    mFormat.vertex( loadResource( "basic.vert" ) )
    .fragment( loadResource( "basic.frag" ) );
    mShader = gl::GlslProg::create( mFormat );

    // Load ground
    mGroundContent.load(mShader);
}

void SampleRenderable::render(const ci::Vec2i & screenOffset)
{
    render(screenOffset, 1.0f);
}

void SampleRenderable::render(const ci::Vec2i & screenOffset, const float alpha)
{
//    gl::clear( ColorAf( 0, 0, 0, 0.5f) );
    
    // Bind shader
    mShader->bind();
    gl::setMatrices( mCam );
    
    // Draw ground
    gl::pushMatrices();

//    gl::enableAlphaBlending();
    
    gl::setDefaultShaderVars();
    // float linearProgress = std::min<float>((float)mNumFramesRendered / (kFramesFullTransition*10.0f), 1.0f);
    mShader->uniform("uColor", ColorAf(0.75,0.75,0.75, alpha));
    
    // Get the current plot
    float groundScale = mGroundContent.getScale();
    Vec3f groundOffset((-0.5f * groundScale),
                       0,
                       (-0.5f * groundScale));
    
    mGroundContent.render(GL_LINES, groundOffset);
    
    gl::popMatrices();
    
    // Draw cube(s)
    gl::pushMatrices();
    // Translate if necessary
    gl::setDefaultShaderVars();
    
    gl::drawCube(Vec3f(0,100,0), Vec3f(200,200,200));
    
    gl::popMatrices();
    
    mShader->unbind();
}

void SampleRenderable::update(std::function<void (ci::CameraPersp & cam)> update_func)
{
    update_func(mCam);
}

ci::Camera & SampleRenderable::getCamera()
{
    return mCam;
}
