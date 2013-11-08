//
//  OpponentGeometry.h
//  BigScreensComposite
//
//  Created by Ryan Bartley on 11/8/13.
//
//

#pragma once

#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/Trimesh.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/Texture.h"

namespace bigscreens {

class PyramidalGeometry;
class SphericalGeometry;
class DynamicOpponent;
typedef std::shared_ptr<PyramidalGeometry>	PyramidalGeometryRef;
typedef std::shared_ptr<SphericalGeometry>	SphericalGeometryRef;
typedef std::shared_ptr<DynamicOpponent>	DynamicOpponentRef;
typedef PyramidalGeometryRef				MinionRef;

class DynamicOpponent {
public:
	DynamicOpponent();
	~DynamicOpponent() {}
	
	void render();
	
private:
	void loadShaders();
	void loadBuffers();
	void loadGeometry();
	void update();
	void draw();
	
private:
	ci::gl::VaoRef mVao[2];
	ci::gl::VboRef mPositionVbo[2];
	ci::gl::TextureRef mNoiseTexture;
	ci::gl::GlslProgRef mGlsl;
	ci::TriMeshRef	mTrimesh;
	PyramidalGeometryRef	mPyramid;
	SphericalGeometryRef	mSphere;
};


class PyramidalGeometry {
public:
	PyramidalGeometry() : mPyramidalTrimesh( ci::TriMesh::create( ci::TriMesh::Format().positions(3).normals() ) )
	{
		calcGeometry();
		loadBuffers();
	}
	~PyramidalGeometry(){}
	
	void draw();
	
	void bindTexBuffer()
	{
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_BUFFER, mTexBuffer );
	}
	
	void unbindTexBuffer()
	{
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_BUFFER, 0 );
	}
	
private:
	
	// SETUP FUNCTIONS
	void loadBuffers();
	void calcGeometry();
	
private:
	// PRIVATE MEMBERS
	
	ci::gl::VaoRef mPyramidalVao;
	ci::gl::VboRef mPyramidalVbo, mPyramidalNormalVbo, mPyramidalElementVbo;
	ci::TriMeshRef mPyramidalTrimesh;
	GLuint			mTexBuffer;
};
	
class SphericalGeometry {
public:
	SphericalGeometry() : mSphericalTrimesh( ci::TriMesh::create( ci::TriMesh::Format().positions(3).normals() ) )
	{
		calcGeometry( 1, 20, 20 );
		loadBuffers();
	}
	~SphericalGeometry() {}
	
	void draw();
	
	void bindTexBuffer()
	{
		glActiveTexture( GL_TEXTURE1 );
		glBindTexture( GL_TEXTURE_BUFFER, mTexBuffer );
	}
	
	void unbindTexBuffer()
	{
		glActiveTexture( GL_TEXTURE1 );
		glBindTexture( GL_TEXTURE_BUFFER, 0 );
	}
	
private:
	// SETUP FUNCTIONS
	void loadBuffers();
	void calcGeometry( float radius, unsigned int rings, unsigned int sectors );
	
private:
	// PRIVATE MEMBERS
	
	ci::gl::VboRef	mSphericalVbo, mSphericalNormalVbo, mSphericalElementVbo;
	ci::gl::VaoRef	mSphericalVao;
	ci::TriMeshRef	mSphericalTrimesh;
	GLuint			mTexBuffer;
};


	
}
