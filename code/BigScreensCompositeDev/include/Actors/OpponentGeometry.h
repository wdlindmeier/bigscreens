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

class MinionGeometry;
class SphericalGeometry;
class DynamicOpponent;
typedef std::shared_ptr<MinionGeometry>	    MinionGeometryRef;
typedef std::shared_ptr<SphericalGeometry>	SphericalGeometryRef;
typedef std::shared_ptr<DynamicOpponent>	DynamicOpponentRef;

class DynamicOpponent {
public:
	DynamicOpponent();
	~DynamicOpponent() {}
	
	void update( float percentage );
	void draw( const ci::Vec3f & cameraView );
	
private:
	void loadShaders();
	void loadBuffers();
	void loadTexture();
	
private:
	
	SphericalGeometryRef	mPyramid;
	SphericalGeometryRef	mSphere;
	
	ci::gl::VaoRef mVao[2];
	ci::gl::VboRef mPositionVbo[2], mAnimatingVbo[2], mTimeLeft[2], mElementVbo;
	ci::gl::TextureRef mNoiseTexture;
	ci::gl::GlslProgRef mRenderOppDynamicGlsl, mUpdateOppDynamicGlsl;
	ci::TriMeshRef	mTrimesh;
	GLuint			mRenderSub, mUpdateSub, mTFOs[2], drawBuf;
};
	
class SphericalGeometry {
public:
	SphericalGeometry( bool sphere ) : mSphericalTrimesh( ci::TriMesh::create( ci::TriMesh::Format().positions(3).normals() ) )
	{
		calcGeometry( 1, 20, 20, sphere );
		loadBuffers();
	}
	~SphericalGeometry() {}
	
	void draw();
	
	void bindTexBuffer( GLenum texturebind )
	{
		glActiveTexture( texturebind );
		glBindTexture( GL_TEXTURE_BUFFER, mTexBuffer );
	}
	
	void unbindTexBuffer( GLenum texturebind )
	{
		glActiveTexture( texturebind );
		glBindTexture( GL_TEXTURE_BUFFER, 0 );
	}
	
	ci::TriMeshRef getTrimesh() { return mSphericalTrimesh; }
	
private:
	// SETUP FUNCTIONS
	void loadBuffers();
	void calcGeometry( float radius, unsigned int rings, unsigned int sectors, bool sphere );
	
private:
	// PRIVATE MEMBERS
	
	ci::gl::VboRef	mSphericalVbo, mSphericalNormalVbo, mSphericalElementVbo;
	ci::gl::VaoRef	mSphericalVao;
	ci::TriMeshRef	mSphericalTrimesh;
	GLuint			mTexBuffer;
};

class MinionGeometry {
public:
	MinionGeometry() : mPyramidalTrimesh( ci::TriMesh::create( ci::TriMesh::Format().positions(3).normals() ) )
	{
		calcGeometry();
		loadBuffers();
		loadShaders();
	}
	~MinionGeometry(){}
	
	// LightPosition is the camera's position in the world
	void draw( const ci::Vec3f & lightPosition, const ci::ColorA & minionColor  );
	
	ci::TriMeshRef getTrimesh() { return mPyramidalTrimesh; }
	
private:
	
	// SETUP FUNCTIONS
	void loadBuffers();
	void calcGeometry();
	void loadShaders();
	
private:
	// PRIVATE MEMBERS
	
	ci::gl::VaoRef mPyramidalVao;
	ci::gl::VboRef mPyramidalVbo, mPyramidalNormalVbo, mPyramidalElementVbo;
	ci::TriMeshRef mPyramidalTrimesh;
	ci::gl::GlslProgRef mGlsl;
};
	
}
