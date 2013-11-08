//
//  PerlinContent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/25/13.
//
//

#include "PerlinContent.h"
#include "cinder/Utilities.h"
#include "cinder/Camera.h"
#include "cinder/gl/Shader.h"
#include "Utilities.hpp"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace bigscreens;

namespace bigscreens
{
    
    PerlinContent::PerlinContent() :
    mSeed(clock() & 65535),
	mOctaves(4),
    mPosition(0,0),
	mFrequency(1.0f / 20.0f),
    mPerlin( mOctaves, mSeed )
    {
        Vec2i noiseSize(128,128);
        mNoiseSurface = Surface( noiseSize.x, noiseSize.y, false);
        mTexture = gl::TextureRef( new gl::Texture(noiseSize.x, noiseSize.y) );
    }
    
    void PerlinContent::reset()
    {
        mPosition = Vec2f::zero();
    }
    
    void PerlinContent::update(const ci::Vec2f & move)
    {
        mPosition += move;
        generateNoiseForPosition(mPosition);
    }

    void PerlinContent::generateNoiseForPosition(const ci::Vec2f & position)
    {
        Surface::Iter iter = mNoiseSurface.getIter();
        while( iter.line() )
        {
            while( iter.pixel() )
            {
                // Values are -0.5 .. 0.5
                float v = mPerlin.noise(((iter.x() + position.x) * mFrequency),
                                        ((iter.y() + position.y) * mFrequency));
                float val = 0.5 + v;
                iter.r() = iter.g() = iter.b() = ci::math<int>::clamp(val * 255, 0, 255);
            }
        }
    }
    
    ci::gl::TextureRef PerlinContent::getTextureRef()
    {
        gl::Texture *tex = new gl::Texture(mNoiseSurface.getWidth(), mNoiseSurface.getHeight());
        tex->update(mNoiseSurface);
        return gl::TextureRef(tex);
    }

    void PerlinContent::preRender()
    {
        mTexture->update(mNoiseSurface);
    }
    
    void PerlinContent::render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect)
    {
        preRender();
        gl::bindStockShader(gl::ShaderDef().color());
        gl::clear( ColorAf( 1.0f, 0.0f, 0.0f, 0.0f ) );
        Rectf screenRect(0, 0, getWindowWidth(), getWindowHeight());
        gl::draw(mTexture, screenRect);
    }
	
}
