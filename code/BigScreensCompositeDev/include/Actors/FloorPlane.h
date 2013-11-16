//
//  FloorPlane.h
//  ADJACENCY
//
//  Created by Ryan Bartley on 11/4/13.
//
//

#pragma once

#include "cinder/gl/Vao.h"
#include "cinder/gl/Vbo.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/GlslProg.h"
#include "Utilities.hpp"

namespace bigscreens {
	
typedef std::shared_ptr<class FloorPlane> FloorPlaneRef;

    class FloorPlane {
    
public:

    FloorPlane(const ci::Vec2i & size);
	~FloorPlane(){}
	
    void draw(const long framesRendered,
              const bool shouldRenderColor,
              const ci::ColorAf & colorOutline = ci::ColorAf::white());

    void setFarLimit( float farLimit );
    void setNearLimit( float nearLimit );
    void setNoiseTexture(ci::gl::TextureRef & tex);

private:
        
	void loadTexture();
	void loadShaders();
	void createAndLoadGeometry();

    ci::gl::VaoRef		mVao;
	ci::gl::VboRef		mVbo, mLineElementVbo;
	ci::gl::GlslProgRef mQuadOutlineGlsl, mQuadTriangleGlsl;
	ci::TriMeshRef		mTrimesh;
	ci::gl::TextureRef  mNoiseTexture;
	int					mNearLimit, mFarLimit;
    const ci::Vec2i     mSize;
    const int           mIndexCount;
};
	
}