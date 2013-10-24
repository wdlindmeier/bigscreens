//
//  TextureContent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/24/13.
//
//

#include "cinder/Cinder.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "SharedTypes.hpp"
#include "cinder/TriMesh.h"

namespace bigscreens
{
    
    class TextureContent : public bigscreens::RenderableContent
    {
        
    public:
        
        TextureContent();
        ~TextureContent(){};
        
        void load(const std::string & textureName);
        void render(const ci::Vec2i & screenOffset);
        ci::CameraPersp& getCamera() { return mCam; }
        
    private:
        
        ci::gl::TextureRef  mTexture;
        ci::CameraPersp mCam;
        
    };
}