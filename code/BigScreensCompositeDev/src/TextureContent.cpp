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
        gl::Texture::Format texFormat;
        texFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR_MIPMAP_LINEAR ).mipMap().internalFormat( GL_RGBA );
        texFormat.maxAnisotropy(gl::Texture::getMaxMaxAnisotropy() );
        mTexture = gl::TextureRef(new gl::Texture(loadImage(app::loadResource(textureName)), texFormat));
    }
    
    void TextureContent::render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect)
    {
        gl::pushMatrices();
        gl::bindStockShader(gl::ShaderDef().color());
        gl::setMatricesWindow(getWindowSize());
        gl::enableAlphaBlending();
        gl::color(1,1,1,1);
        gl::setDefaultShaderVars();
        Rectf screenRect(0, 0, getWindowWidth(), getWindowHeight());
        gl::draw(mTexture, screenRect);
        gl::disableAlphaBlending();
        gl::popMatrices();
    }
	
}
