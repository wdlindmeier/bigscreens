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
        Surface::Iter iter = mNoiseSurface.getIter();
        while( iter.line() )
        {
            while( iter.pixel() )
            {
                float v = mPerlin.noise(((iter.x() + mPosition.x) * mFrequency) + 1,
                                        ((iter.y() + mPosition.y) * mFrequency) + 1);
                // v *= v * v;
                //uint8_t val = v;// * 255;
                float val = v * 5;
                iter.r() = iter.g() = iter.b() = val;
            }
        }
        /*
        float v = mPerlin.noise((float)mPosition.x, (float)mPosition.y);
        v *= 1000;
        ci::app::console() << "up " << v << "\n";
        */
    }
    
    void PerlinContent::render(const ci::Vec2i & screenOffset)
    {
        mTexture->update(mNoiseSurface);
        gl::bindStockShader(gl::ShaderDef().color());
        gl::clear( ColorAf( 1.0f, 0.0f, 0.0f, 0.0f ) );
        Rectf screenRect(0, 0, getWindowWidth(), getWindowHeight());
        gl::draw(mTexture, screenRect);
    }
	
}
