//
//  GroundContent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 10/31/13.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Camera.h"
#include "cinder/gl/Texture.h"
#include "SharedTypes.hpp"
#include "cinder/Perlin.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Surface.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Fbo.h"
#include "cinder/TriMesh.h"

namespace bigscreens
{
    
    class GroundContent
    {
        
    public:
        
        GroundContent(float scale, float depthScale = 0.075f);
        ~GroundContent(){};
        
        void load(ci::gl::GlslProgRef & useShader);
        void render(GLenum drawMode, const ci::Vec3f & offset);
        float getScale(){ return mGroundPlaneScale; };

    private:

        ci::CameraPersp mCam;
        
        // NOTE: The coords need to be scalar to map to the
        // height map. The matrix is then scaled up in draw
        // to fit the scene.
        float mGroundPlaneDimension;
        float mGroundPlaneUnitScale;
        float mGroundPlaneScale;
        float mGroundDepthScale;
        float mGroundPlaneUnit;
        int   mNumGroundUnitsWide;
        int   mNumGroundUnitsHigh;
        float mGroundY;
        int   mNumPlots;
        int   mVertsPerPlot;
        int   mNumVerts;
        
        ci::gl::VaoRef      mGroundVao;
        ci::gl::VboRef      mGroundVbo;
    };
}
