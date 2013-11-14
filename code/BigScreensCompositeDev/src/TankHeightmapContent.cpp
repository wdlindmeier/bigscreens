//
//  TankHeightmapContent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/24/13.
//
//

#include "TankHeightmapContent.h"

using namespace ci;
using namespace std;

namespace bigscreens
{
    
    TankHeightmapContent::TankHeightmapContent() :
    TankContent(),
    mGroundContent(5000.0)
    {
        //mGroundContent = GroundContent(5000.0);
    };

    void TankHeightmapContent::loadGround()
    {
        assert(mHeightmapShader);
        mGroundContent.load(mHeightmapShader);
    }

    void TankHeightmapContent::loadShaders()
    {
        TankContent::loadShaders();        
        gl::GlslProg::Format shaderFormat;
        shaderFormat.vertex( ci::app::loadResource( "heightmap.vert" ) )
        .fragment( ci::app::loadResource( "heightmap.frag" ) );
        mHeightmapShader = ci::gl::GlslProg::create( shaderFormat );
    }
    
    void TankHeightmapContent::load(const std::string & objFilename)
    {
        TankContent::load(objFilename);
        loadGround();
        mPerlinContent.reset();
    }
    
    void TankHeightmapContent::generateGroundMaps()
    {
        // TODO: Optimize this so we don't generate textures we already have
        // in the vector. Only generate new textures.
        mGroundMaps.clear();
        
        Vec2i texSize = mPerlinContent.getTexture()->getSize();
        
        for (int i = 0; i < 9; ++i)
        {
            int x = (i % 3) - 1;
            int z = ((i / 3) - 1) * -1;
            int plotX = mPlotCoords.x + x;
            int plotZ = mPlotCoords.z + z;
            
            mPerlinContent.generateNoiseForPosition(Vec2f(texSize.x * plotX,
                                                          texSize.y * plotZ * -1));
            gl::TextureRef newTex = mPerlinContent.getTextureRef();
            mGroundMaps.push_back(newTex);
        }
    }
    
    void TankHeightmapContent::drawGround()
    {
        // Get the current plot
        float groundScale = mGroundContent.getScale();
        int plotX = (mTankPosition.x + (groundScale * 0.5f)) / groundScale;
        int plotZ = (mTankPosition.z + (groundScale * 0.5f)) / groundScale;
        
        if (mPlotCoords.x != plotX || mPlotCoords.z != plotZ || mGroundMaps.size() == 0)
        {
            mPlotCoords = Vec3i(plotX, 0, plotZ);
            generateGroundMaps();
        }

        drawGroundTile(mPlotCoords + Vec3i(-1, 0, 1), mGroundMaps[0]);
        drawGroundTile(mPlotCoords + Vec3i(0, 0, 1), mGroundMaps[1]);
        drawGroundTile(mPlotCoords + Vec3i(1, 0, 1), mGroundMaps[2]);

        drawGroundTile(mPlotCoords + Vec3i(-1, 0, 0), mGroundMaps[3]);
        drawGroundTile(mPlotCoords + Vec3i(0, 0, 0), mGroundMaps[4]);
        drawGroundTile(mPlotCoords + Vec3i(1, 0, 0), mGroundMaps[5]);

        drawGroundTile(mPlotCoords + Vec3i(-1, 0, -1), mGroundMaps[6]);
        drawGroundTile(mPlotCoords + Vec3i(0, 0, -1), mGroundMaps[7]);
        drawGroundTile(mPlotCoords + Vec3i(1, 0, -1), mGroundMaps[8]);
    }

    void TankHeightmapContent::drawGroundTile(const ci::Vec3i & plot, gl::TextureRef & heightMap)
    {
        float groundScale = mGroundContent.getScale();

        Vec3f positionOffset = plot * groundScale;

        gl::pushMatrices();
        
        gl::setMatrices( mCam );

        mHeightmapShader->bind();
        
        heightMap->bind(0);
        mHeightmapShader->uniform("uTex0", 0);
        
        Vec3f groundOffset((-0.5f * groundScale) + positionOffset.x,
                           0 + positionOffset.y,
                           (-0.5f * groundScale) + positionOffset.z);

        mGroundContent.render(GL_LINES, groundOffset);

        mHeightmapShader->unbind();
        
        gl::popMatrices();
    }

}