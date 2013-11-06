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

namespace bigscreens {

class SampleRenderable : public RenderableContent
{
    
public:
    
    SampleRenderable();
    ~SampleRenderable(){};
    
    void load();
    void render(const ci::Vec2i & screenOffset);
    void update(std::function<void (ci::CameraPersp & cam)> update_func);
    ci::Camera & getCamera();
    
private:
    
    ci::CameraPersp mCam;
    GroundContent mGroundContent;
    ci::gl::GlslProgRef mShader;
};

}