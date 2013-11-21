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
              const float alpha,
              const float mountainMultiplier,
              const ci::Vec3f & groundScale = ci::Vec3f::zero(),
              const ci::Vec3f & groundOffset = ci::Vec3f::zero(),
              const ci::Vec3f & tankPosition = ci::Vec3f::zero(),
              const ci::Vec3f & tankVector = ci::Vec3f::zero());

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