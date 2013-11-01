//
//  GroundContent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/31/13.
//
//

#include "GroundContent.h"

using namespace bigscreens;
using namespace ci;
using namespace ci::app;

GroundContent::GroundContent(float scale, float depthScale) :
mGroundPlaneDimension(1.0f),
mGroundPlaneUnitScale(1.0f/32.0f),
mGroundPlaneScale(scale),
mGroundDepthScale(depthScale),
mGroundPlaneUnit(mGroundPlaneDimension * mGroundPlaneUnitScale),
mNumGroundUnitsWide(mGroundPlaneDimension / mGroundPlaneUnit),
mNumGroundUnitsHigh(mGroundPlaneDimension / mGroundPlaneUnit),
mGroundY(0.0f),
mNumPlots(mNumGroundUnitsWide * mNumGroundUnitsHigh),
// NOTE: This could be changed for a different geometry.
// Splitting each plot into 2 triangles for now.
mVertsPerPlot(4),
mNumVerts(mNumPlots * mVertsPerPlot)
{
}

void GroundContent::load(ci::gl::GlslProgRef & useShader)
{
    GLfloat verts[mNumPlots * mVertsPerPlot * 3];
    
    for (int z = 0; z < mNumGroundUnitsHigh; ++z)
    {
        for (int x = 0; x < mNumGroundUnitsWide; ++x)
        {
            int plotIndex = (z * mNumGroundUnitsWide) + x;
            int idxOffset = plotIndex * mVertsPerPlot * 3; //xyz
            
            //1: TL
            verts[idxOffset + (0 * 3 + 0)] = (x * mGroundPlaneUnit);
            verts[idxOffset + (0 * 3 + 1)] = mGroundY;
            verts[idxOffset + (0 * 3 + 2)] = (z * mGroundPlaneUnit);
            
            //2: TR
            verts[idxOffset + (1 * 3 + 0)] = ((x + 1) * mGroundPlaneUnit);
            verts[idxOffset + (1 * 3 + 1)] = mGroundY;
            verts[idxOffset + (1 * 3 + 2)] = (z * mGroundPlaneUnit);
            
            //3: BL
            verts[idxOffset + (2 * 3 + 0)] = (x * mGroundPlaneUnit);
            verts[idxOffset + (2 * 3 + 1)] = mGroundY;
            verts[idxOffset + (2 * 3 + 2)] = ((z + 1) * mGroundPlaneUnit);
            
            //4: BR
            verts[idxOffset + (3 * 3 + 0)] = ((x + 1) * mGroundPlaneUnit);
            verts[idxOffset + (3 * 3 + 1)] = mGroundY;
            verts[idxOffset + (3 * 3 + 2)] = ((z + 1) * mGroundPlaneUnit);
        }
    }
    
    mGroundVao = gl::Vao::create();
    mGroundVao->bind();
    mGroundVbo = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW );
    mGroundVbo->bind();
    
    int posLoc = useShader->getAttribSemanticLocation( geom::Attrib::POSITION );
    gl::enableVertexAttribArray( posLoc );
    gl::vertexAttribPointer( posLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    
    mGroundVbo->unbind();
    mGroundVao->unbind();
}

void GroundContent::render(GLenum drawMode, const ci::Vec3f & offset)
{
    gl::pushMatrices();
    gl::translate(offset);
    // 0..1 -> 0..5000
    gl::scale(mGroundPlaneScale,
              mGroundPlaneScale * mGroundDepthScale,
              mGroundPlaneScale);

    mGroundVao->bind();
    mGroundVbo->bind();
    
    gl::setDefaultShaderVars();
    
    gl::drawArrays(drawMode, 0, mNumVerts);
    
    mGroundVao->unbind();
    mGroundVbo->unbind();
    gl::popMatrices();
}
