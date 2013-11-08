//
//  OppDynamicGeom.hpp
//  Opponent
//
//  Created by Ryan Bartley on 11/6/13.
//
//

#pragma once

#include "PyramidalGeometry.hpp"
#include "SphericalGeometry.hpp"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/Texture.h"

namespace bigscreens {
	
class DynamicOpponent {
public:
	DynamicOpponent()
	{
		mPyramid = PyramidalGeometryRef( new PyramidalGeometry() );
		mSphere = SphericalGeometryRef( new SphericalGeometry() );
		
		ci::TriMesh::Format mFormat;
		mFormat.positions(3);
		
		mTrimesh = ci::TriMesh::create( mFormat );
		
		loadShaders();
		loadGeometry();
		loadBuffers();
	}
	~DynamicOpponent() {}
	
	void render();
	
private:
	void loadShaders()
	{
		const char * varyings[] = {
			"position"
		};
		
		ci::gl::GlslProg::Format mGlslFormat;
		mGlslFormat.vertex( ci::app::loadAsset( "oppDynamic.vert" ) )
			.geometry( ci::app::loadAsset( "oppDynamic.geom" ) )
			.fragment( ci::app::loadAsset( "oppDynamic.geom" ) )
			.transformFeedback().feedbackVaryings( varyings, 1 )
			.feedbackFormat( GL_SEPARATE_ATTRIBS );
		
	}
	
	void loadBuffers()
	{
		
	}
	
	void loadGeometry()
	{
		for( int i = 0; i < 1200; i++ ) {
			mTrimesh->appendVertex( ci::Vec3i( 0.0f, 0.0f, 0.0f ) );
		}
		
//		mTrimesh->appendIndices( , );
	}
	
	void update()
	{
		
	}
	
	void draw()
	{
		
	}
	
private:
	ci::gl::VaoRef mVao[2];
	ci::gl::VboRef mPositionVbo[2];
	ci::gl::TextureRef mNoiseTexture;
	ci::gl::GlslProgRef mGlsl;
	ci::TriMeshRef	mTrimesh;
	PyramidalGeometryRef	mPyramid;
	SphericalGeometryRef	mSphere;
};
	
}