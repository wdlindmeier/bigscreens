//
//  SampleRenderable.h
//  ConvergenceTest
//
//  Created by William Lindmeier on 11/6/13.
//
//

#pragma once

#include "SharedTypes.hpp"
#include "GroundContent.h"
#include "cinder/gl/GlslProg.h"

const static int kFramesFullTransition = 500;

namespace bigscreens {

class SampleRenderable : public RenderableContent
{
    
public:
    
    SampleRenderable();
    ~SampleRenderable(){};
    
    void load();
    void render(const ci::Vec2i & screenOffset);
    void render(const ci::Vec2i & screenOffset, const float alpha);
    void update(std::function<void (ci::CameraPersp & cam)> update_func);
    ci::Camera & getCamera();
    
private:
    
    ci::CameraPersp mCam;
    GroundContent mGroundContent;
    ci::gl::GlslProgRef mShader;
};

}