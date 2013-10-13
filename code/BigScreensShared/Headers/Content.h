//
//  Content.h
//  CinderProject
//
//  Created by Ryan Bartley on 10/4/13.
//
//

#pragma once

#include "cinder/Camera.h"

using namespace ci;

// Interface for creating windows

namespace bigscreens {

class Content {
public:
	Content() {}
	virtual ~Content() {}
	
	// This is to manipulate the aspect ratio
	// so that we can use different windows
	// for our content
	virtual CameraPersp& getCamera() = 0;
	
	// This is what every renderable class
	// should have
	virtual void render() = 0;
	
private:
	
};

}