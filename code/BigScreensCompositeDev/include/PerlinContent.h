//
//  PerlinContent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/25/13.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Camera.h"
#include "cinder/gl/Texture.h"
#include "SharedTypes.hpp"
#include "cinder/Perlin.h"
#include "cinder/Surface.h"

namespace bigscreens
{
    
    class PerlinContent : public bigscreens::RenderableContent
    {
        
    public:
        
        PerlinContent();
        ~PerlinContent(){};
        
        void update(const ci::Vec2f & move);
        void generateNoiseForPosition(const ci::Vec2f & position);
        float getValueAtPosition(const ci::Vec2f & position);
        void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
        void preRender();
        ci::gl::TextureRef getTextureRef();
        void reset();
        ci::CameraPersp& getCamera() { return mCam; }
        ci::gl::TextureRef getTexture(){ return mTexture; };
        
    private:
        
        ci::gl::TextureRef      mTexture;
        ci::CameraPersp         mCam;
        ci::Surface             mNoiseSurface;
        int						mSeed;
        int						mOctaves;
        ci::Vec2f               mPosition;
        ci::Perlin				mPerlin;
        float					mFrequency;
        
    };
}