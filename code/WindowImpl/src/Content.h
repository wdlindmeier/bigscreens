//
//  Content.h
//  CinderProject
//
//  Created by Ryan Bartley on 10/4/13.
//
//

#pragma once

#include <map>
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Matrix44.h"

using namespace ci;

namespace bigscreens {

class Content {
public:
	Content( ) {}
	virtual ~Content() {}
	
	virtual CameraPersp& getCamera() = 0;
	
	virtual void render() = 0;
	
private:
	
};

}