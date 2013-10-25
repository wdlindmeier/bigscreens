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
    const float kGroundPlaneScale = 10000.0f;
    const float kGroundDepthScale = 0.075f;
    const float kGroundPlaneUnit = kGroundPlaneDimension * kGroundPlaneUnitScale;
    const int   kNumGroundUnitsWide = kGroundPlaneDimension / kGroundPlaneUnit;
    const int   kNumGroundUnitsHigh = kGroundPlaneDimension / kGroundPlaneUnit;
    const float kGroundZ = 0.0f;
    const int   kNumPlots = kNumGroundUnitsWide * kNumGroundUnitsHigh;
    // NOTE: This could be changed for a different geometry.
    // Splitting each plot into 2 triangles for now.
    const int   kVertsPerPlot = 4;
    const int   kNumVerts = kNumPlots * kVertsPerPlot;
    
    void TankHeightmapContent::loadGround()
    {
        mHeightmapTexture = gl::TextureRef(new gl::Texture(loadImage(app::loadResource("height_sample.png"))));

        gl::Texture::Format texFormat;
        texFormat.setWrap(GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T);
        
        GLfloat verts[kNumPlots * kVertsPerPlot * 3];
        
        for (int y = 0; y < kNumGroundUnitsHigh; ++y)
        {
            for (int x = 0; x < kNumGroundUnitsWide; ++x)
            {
                int plotIndex = (y * kNumGroundUnitsWide) + x;
                int idxOffset = plotIndex * kVertsPerPlot * 3; //xyz
                
                //1: TL
                verts[idxOffset + (0 * 3 + 0)] = (x * kGroundPlaneUnit);
                verts[idxOffset + (0 * 3 + 1)] = (y * kGroundPlaneUnit);
                verts[idxOffset + (0 * 3 + 2)] = kGroundZ;
                
                //2: TR
                verts[idxOffset + (1 * 3 + 0)] = ((x + 1) * kGroundPlaneUnit);
                verts[idxOffset + (1 * 3 + 1)] = (y * kGroundPlaneUnit);
                verts[idxOffset + (1 * 3 + 2)] = kGroundZ;
                
                //3: BL
                verts[idxOffset + (2 * 3 + 0)] = (x * kGroundPlaneUnit);
                verts[idxOffset + (2 * 3 + 1)] = ((y + 1) * kGroundPlaneUnit);
                verts[idxOffset + (2 * 3 + 2)] = kGroundZ;
                
                //4: BR
                verts[idxOffset + (3 * 3 + 0)] = ((x + 1) * kGroundPlaneUnit);
                verts[idxOffset + (3 * 3 + 1)] = ((y + 1) * kGroundPlaneUnit);
                verts[idxOffset + (3 * 3 + 2)] = kGroundZ;
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
    
    void TankHeightmapContent::drawGround()
    {
        gl::pushMatrices();
        
        gl::setMatrices( mCam );
        
        // Align with tank :-/
        // We should probably rotate the tank instead.
        gl::rotate(90, 1, 0, 0);
        gl::rotate(90, 0, 0, 1);
        
        // Center the ground
        gl::translate(Vec3f(-0.5f * kGroundPlaneScale,
                            -0.5f * kGroundPlaneScale,
                            0));
        

        // 0..1 -> 0..5000
        gl::scale(kGroundPlaneScale,
                  kGroundPlaneScale,
                  kGroundPlaneScale * kGroundDepthScale);

        mHeightmapShader->bind();
                
        // Set the height texture
        mHeightmapTexture->bind(0);
        mHeightmapShader->uniform("uTex0", 0);
        

        mGroundVao->bind();
        mGroundVbo->bind();
        
        gl::setDefaultShaderVars();
        gl::drawArrays(GL_LINES, 0, kNumVerts);
        
        mGroundVao->unbind();
        mGroundVbo->unbind();
        
        mHeightmapShader->unbind();
        gl::popMatrices();
    }

}