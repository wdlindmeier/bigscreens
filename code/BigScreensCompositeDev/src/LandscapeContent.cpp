//
//  LandscapeContent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/27/13.
//
//

#include "LandscapeContent.h"
#include "cinder/ObjLoader.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include "cinder/gl/Shader.h"
#include "Utilities.hpp"

using namespace ci;
using namespace ci::app;
using namespace bigscreens;

namespace bigscreens
{
    
    LandscapeContent::LandscapeContent() :
    mInitialOffset(0.0f,-500.0f)
    ,mScrollVector(0,-0.5)
    {
    }
    
    void LandscapeContent::setInitialOffset(const ci::Vec2i & offset)
    {
        mInitialOffset = offset;
    }
    
    void LandscapeContent::setScrollVector(const ci::Vec2f & vector)
    {
        mScrollVector = vector;
    }
    
    void LandscapeContent::load(const std::string & textureName)
    {
        gl::Texture::Format texFormat;
        texFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR_MIPMAP_LINEAR ).mipMap().internalFormat( GL_RGBA );
        texFormat.maxAnisotropy(gl::Texture::getMaxMaxAnisotropy() );
        mTexture = gl::TextureRef(new gl::Texture(loadImage(app::loadResource(textureName)), texFormat));
    }
    
    void LandscapeContent::render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect)
    {
        gl::pushMatrices();
        gl::bindStockShader(gl::ShaderDef().color());
        gl::setMatricesWindow(contentRect.getSize());
        gl::disableAlphaBlending();
        gl::color(1,1,1,1);
        gl::setDefaultShaderVars();
        gl::translate(mInitialOffset + (mScrollVector * mNumFramesRendered));
        gl::draw(mTexture);
        gl::popMatrices();
    }	
}
