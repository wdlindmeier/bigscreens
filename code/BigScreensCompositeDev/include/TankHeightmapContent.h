//
//  TankHeightmapContent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/24/13.
//
//

#pragma once

#include "TankContent.h"
#include "PerlinContent.h"
#include "GroundContent.h"

namespace bigscreens
{
    class TankHeightmapContent : public TankContent
    {
        
    public:
        
        TankHeightmapContent();
        virtual ~TankHeightmapContent(){};
        
        virtual void load(const std::string & objFilename);

    protected:
        
        virtual void loadGround();
        virtual void loadShaders();

        virtual void drawGround();

        // Ground plane
        ci::gl::GlslProgRef             mHeightmapShader;
        GroundContent                   mGroundContent;

        void generateGroundMaps();
        void drawGroundTile(const ci::Vec3i & plot, ci::gl::TextureRef & heightMap);

        PerlinContent                   mPerlinContent;
        ci::Vec3i                       mPlotCoords;
        std::vector<ci::gl::TextureRef> mGroundMaps;
    };
}