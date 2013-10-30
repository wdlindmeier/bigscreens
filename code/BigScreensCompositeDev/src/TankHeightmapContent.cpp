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
    // NOTE: The coords need to be scalar to map to the
    // height map. The matrix is then scaled up in draw
    // to fit the scene.
    const float kGroundPlaneDimension = 1.0f;
    // NOTE: The heightmap is 64px wide.
    // This can/should change depending upon the texture.
    const float kGroundPlaneUnitScale = 1.0f/32.0f;//64.0f;
    const float kGroundPlaneScale = 5000.0f;
    const float kGroundDepthScale = 0.075f;
    const float kGroundPlaneUnit = kGroundPlaneDimension * kGroundPlaneUnitScale;
    const int   kNumGroundUnitsWide = kGroundPlaneDimension / kGroundPlaneUnit;
    const int   kNumGroundUnitsHigh = kGroundPlaneDimension / kGroundPlaneUnit;
    const float kGroundY = 0.0f;
    const int   kNumPlots = kNumGroundUnitsWide * kNumGroundUnitsHigh;
    // NOTE: This could be changed for a different geometry.
    // Splitting each plot into 2 triangles for now.
    const int   kVertsPerPlot = 4;
    const int   kNumVerts = kNumPlots * kVertsPerPlot;
    
    void TankHeightmapContent::loadGround()
    {
        gl::Texture::Format texFormat;
        texFormat.setWrap(GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T);
        
        GLfloat verts[kNumPlots * kVertsPerPlot * 3];
        
        for (int z = 0; z < kNumGroundUnitsHigh; ++z)
        {
            for (int x = 0; x < kNumGroundUnitsWide; ++x)
            {
                int plotIndex = (z * kNumGroundUnitsWide) + x;
                int idxOffset = plotIndex * kVertsPerPlot * 3; //xyz
                
                //1: TL
                verts[idxOffset + (0 * 3 + 0)] = (x * kGroundPlaneUnit);
                verts[idxOffset + (0 * 3 + 1)] = kGroundY;
                verts[idxOffset + (0 * 3 + 2)] = (z * kGroundPlaneUnit);
                
                //2: TR
                verts[idxOffset + (1 * 3 + 0)] = ((x + 1) * kGroundPlaneUnit);
                verts[idxOffset + (1 * 3 + 1)] = kGroundY;
                verts[idxOffset + (1 * 3 + 2)] = (z * kGroundPlaneUnit);
                
                //3: BL
                verts[idxOffset + (2 * 3 + 0)] = (x * kGroundPlaneUnit);
                verts[idxOffset + (2 * 3 + 1)] = kGroundY;
                verts[idxOffset + (2 * 3 + 2)] = ((z + 1) * kGroundPlaneUnit);
                
                //4: BR
                verts[idxOffset + (3 * 3 + 0)] = ((x + 1) * kGroundPlaneUnit);
                verts[idxOffset + (3 * 3 + 1)] = kGroundY;
                verts[idxOffset + (3 * 3 + 2)] = ((z + 1) * kGroundPlaneUnit);
            }
        }
        
        mGroundVao = gl::Vao::create();
        mGroundVao->bind();
        mGroundVbo = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW );
        mGroundVbo->bind();
        
        int posLoc = mHeightmapShader->getAttribSemanticLocation( geom::Attrib::POSITION );
        gl::enableVertexAttribArray( posLoc );
        gl::vertexAttribPointer( posLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
        
        mGroundVbo->unbind();
        mGroundVao->unbind();
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
        mPerlinContent.reset();
    }

    void TankHeightmapContent::update(const ci::Vec3f & tankMovement)
    {
        // Update the tank position
        mTankMovement += tankMovement;
        
        // Move the camera with it
        mCam.lookAt(Vec3f( 0, 600, -1000 ) + mTankMovement,
                    Vec3f( 0, 100, 0 ) + mTankMovement);
    }
    
    void TankHeightmapContent::drawTank()
    {
        mTankShader->bind();
        mTankVao->bind();
        
        gl::pushMatrices();
        
        gl::setMatrices( mCam );
        gl::translate(mTankMovement);
        
        mTankShader->uniform( "projection", gl::getProjection() );
        mTankShader->uniform( "modelView", gl::getModelView() );
        
        gl::drawArrays( GL_LINES, 0, mTankMesh->getNumVertices() );
        //gl::drawElements( GL_LINES, mTankMesh->getNumIndices(), GL_UNSIGNED_INT, 0 );
        gl::popMatrices();
        
        mTankVao->unbind();
        mTankShader->unbind();
        //mTankElementVbo->unbind();
        
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
        int plotX = (mTankMovement.x + (kGroundPlaneScale * 0.5)) / kGroundPlaneScale;
        int plotZ = (mTankMovement.z + (kGroundPlaneScale * 0.5)) / kGroundPlaneScale;
        
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
        
        Vec3f positionOffset = plot * kGroundPlaneScale;

        gl::pushMatrices();
        
        gl::setMatrices( mCam );

        gl::translate(Vec3f((-0.5f * kGroundPlaneScale) + positionOffset.x,
                            0 + positionOffset.y,
                            (-0.5f * kGroundPlaneScale) + positionOffset.z)),
        
        // 0..1 -> 0..5000
        gl::scale(kGroundPlaneScale,
                  kGroundPlaneScale * kGroundDepthScale,
                  kGroundPlaneScale);

        mHeightmapShader->bind();

        // Set the height texture
        heightMap->bind(0);
        
        // mHeightmapTexture->bind(0);
        mHeightmapShader->uniform("uTex0", 0);

        mGroundVao->bind();
        mGroundVbo->bind();
        
        gl::setDefaultShaderVars();
        gl::drawArrays(GL_LINES, 0, kNumVerts);
        // gl::drawArrays(GL_LINE_STRIP, 0, kNumVerts);
        
        mGroundVao->unbind();
        mGroundVbo->unbind();
        
        mHeightmapShader->unbind();
        gl::popMatrices();
    }

}