//
//  PerlinContent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/25/13.
//
//

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
        void render(const ci::Vec2i & screenOffset);
        void reset();
        ci::CameraPersp& getCamera() { return mCam; }
        
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