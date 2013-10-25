//
//  TextureContent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/24/13.
//
//

#include "TextureContent.h"

#include "TankContent.h"
#include "cinder/ObjLoader.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include "cinder/Camera.h"
#include "cinder/gl/Shader.h"
#include "Utilities.hpp"

using namespace ci;
using namespace ci::app;
using namespace bigscreens;

namespace bigscreens
{
    
    TextureContent::TextureContent()
    {
    }
    
    void TextureContent::load(const std::string & textureName)
    {
        mTexture = gl::TextureRef(new gl::Texture(loadImage(app::loadResource(textureName))));
    }
    
    void TextureContent::render(const ci::Vec2i & screenOffset)
    {
        gl::bindStockShader(gl::ShaderDef().color());
        
        // clear out both of the attachments of the FBO with black
        gl::clear( ColorAf( 0.0f, 0.0f, 0.0f, 0.0f ) );
        
        gl::enableAdditiveBlending();
        Rectf screenRect(0, 0, getWindowWidth(), getWindowHeight());
        gl::draw(mTexture, screenRect);
        gl::disableAlphaBlending();
    }
	
}
