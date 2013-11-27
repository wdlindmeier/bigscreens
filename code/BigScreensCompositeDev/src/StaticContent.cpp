//
//  StaticContent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/22/13.
//
//

#include "StaticContent.h"
#include "cinder/ObjLoader.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include "cinder/Camera.h"
#include "cinder/gl/Shader.h"
#include "Utilities.hpp"

using namespace ci;
using namespace ci::app;
using namespace bigscreens;

static const int kNumTextures = 3;
static const int kFramesPerTexture = 8;//4;

namespace bigscreens
{    
    StaticContent::StaticContent()
    {
    }
    
    void StaticContent::load()
    {
        gl::Texture::Format texFormat;
        texFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR_MIPMAP_LINEAR ).mipMap().internalFormat( GL_RGBA );
        texFormat.maxAnisotropy(gl::Texture::getMaxMaxAnisotropy() );
        
        for (int i = 0; i < kNumTextures; ++i)
        {
            mTextures.push_back(gl::TextureRef(new gl::Texture(loadImage(app::loadResource("static_" + std::to_string(i) + ".png")),
                                                               texFormat)));
        }
    }
    
    void StaticContent::render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect)
    {
        gl::pushMatrices();
        gl::bindStockShader(gl::ShaderDef().color());
        gl::setMatricesWindow(contentRect.getSize());
        gl::enableAlphaBlending();
        gl::color(1,1,1,1);
        gl::setDefaultShaderVars();
        //Rectf screenRect(0, 0, getWindowWidth(), getWindowHeight());
        gl::TextureRef pickTexture = mTextures[int((float)mNumFramesRendered/(float)kFramesPerTexture) % kNumTextures];
        float scale = std::max(contentRect.getWidth() / (float)pickTexture->getWidth(),
                               contentRect.getHeight() / (float)pickTexture->getHeight());
        gl::draw(pickTexture,
                 Rectf(0, 0, pickTexture->getWidth() * scale,pickTexture->getHeight() * scale));
        gl::disableAlphaBlending();
        gl::popMatrices();
    }
	
}
