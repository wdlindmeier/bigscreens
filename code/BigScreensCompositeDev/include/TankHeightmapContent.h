//
//  TankHeightmapContent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/24/13.
//
//

#pragma once

#include "TankContent.h"

namespace bigscreens
{
    class TankHeightmapContent : public TankContent
    {
        
    public:
        
        TankHeightmapContent() : TankContent() {};
        virtual ~TankHeightmapContent(){};
        
        // virtual void load(const std::string & objFilename);
        // virtual void render(const ci::Vec2i & screenOffset);
        // virtual void reset();
        
    protected:
        
        virtual void loadGround();
        virtual void loadShaders();

        virtual void drawGround();

    private:

        // Ground plane
        ci::gl::GlslProgRef mHeightmapShader;
        ci::gl::TextureRef  mHeightmapTexture;
        ci::gl::VaoRef      mGroundVao;
        ci::gl::VboRef      mGroundVbo;
  
    };
}