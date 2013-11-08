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
	
		// how many across, how many up,  how many units between, element number to create the quads
		//                                                        this was the magic sauce for the elements
const int    xCount = 50,    yCount = 50, quadSize = 10,          indexNum = (xCount-1)*(yCount-1) * 4;

class FloorPlane {
public:
	FloorPlane()
	{
		ci::TriMesh::Format mTriFormat;
		mTriFormat.positions(3);
		mTrimesh = ci::TriMesh::create( mTriFormat );
		
		loadTexture();
		createAndLoadGeometry();
		loadShaders();
	}
	~FloorPlane(){}
	
	void draw();
	
private:
	void loadTexture();
	void loadShaders();
	void createAndLoadGeometry();
	
private:
	ci::gl::VaoRef		mVao;
	ci::gl::VboRef		mVbo, mLineElementVbo;
	ci::gl::GlslProgRef mQuadOutlineGlsl, mQuadTriangleGlsl;
	ci::TriMeshRef		mTrimesh;
	ci::gl::TextureRef  mNoiseTexture;
	bool				mChooseColor;
};
	
}