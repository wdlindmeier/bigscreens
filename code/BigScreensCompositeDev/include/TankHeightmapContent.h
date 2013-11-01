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
        virtual void update(const ci::Vec3f & tankMovement);
        
    protected:
        
        virtual void loadGround();
        virtual void loadShaders();

        void generateGroundMaps();
        virtual void drawGround();
        void drawGroundTile(const ci::Vec3i & plot, ci::gl::TextureRef & heightMap);
        virtual void drawTank();

        //GroundContent       mGroundContent;

        // Ground plane
        ci::gl::GlslProgRef mHeightmapShader;
        ci::Vec3f           mTankMovement;
        
        PerlinContent       mPerlinContent;
        ci::Vec3i           mPlotCoords;
        std::vector<ci::gl::TextureRef> mGroundMaps;

    };
}