//
//  SphericalGeometry.hpp
//  Opponent
//
//  Created by Ryan Bartley on 11/2/13.
//
//

#pragma once

#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/Trimesh.h"

namespace bigscreens {
	
typedef std::shared_ptr<class SphericalGeometry> SphericalGeometryRef;
	
class SphericalGeometry {
public:
	SphericalGeometry()
	{
		ci::TriMesh::Format mSphericalTrimeshFormat;
		mSphericalTrimeshFormat.positions(3).normals();
		mSphericalTrimesh = ci::TriMesh::create( mSphericalTrimeshFormat );
		
		calcGeometry( 1, 20, 20 );
		loadBuffers();
	}
	~SphericalGeometry() {}
	
	void draw() {
		mSphericalVao->bind();
		mSphericalElementVbo->bind();
		
//		ci::gl::drawElements( GL_TRIANGLES, mSphericalTrimesh->getNumIndices(), GL_UNSIGNED_INT, 0);
		ci::gl::drawArrays( GL_TRIANGLES, 0, mSphericalTrimesh->getNumVertices() );
		mSphericalElementVbo->unbind();
		mSphericalVao->unbind();
	}
	
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
	void loadBuffers()
	{
		mSphericalVao = ci::gl::Vao::create();
		mSphericalVao->bind();
		
		mSphericalVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mSphericalTrimesh->getNumVertices() * sizeof( ci::Vec3f ), mSphericalTrimesh->getVertices<3>(), GL_STATIC_DRAW );
		mSphericalVbo->bind();
		ci::gl::enableVertexAttribArray( 0 );
		ci::gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
		
		mSphericalNormalVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mSphericalTrimesh->getNormals().size() * sizeof( ci::Vec3f ), GL_STATIC_DRAW );
		mSphericalNormalVbo->bind();
		ci::gl::enableVertexAttribArray( 1 );
		ci::gl::vertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );
		
		mSphericalElementVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mSphericalTrimesh->getNumIndices() * sizeof( uint32_t ), GL_STATIC_DRAW );
		mSphericalVao->unbind();
		
		glGenTextures( 1, &mTexBuffer );
		glBindTexture( GL_TEXTURE_BUFFER, mTexBuffer );
		glTexBuffer( GL_TEXTURE_BUFFER, GL_RGBA32F, mSphericalVbo->getId() );
	}
	
	void calcGeometry( float radius, unsigned int rings, unsigned int sectors )
	{
		float const R = 1./(float)(rings-1);
		float const S = 1./(float)(sectors-1);
		float r, s;
		
		std::vector< ci::Vec3f > vertices;
		std::vector< ci::Vec3f > normals;
		std::vector< uint32_t > indices;
		
		vertices.resize(rings * sectors * 3);
		normals.resize(rings * sectors * 3);
		std::vector< ci::Vec3f >::iterator v = vertices.begin();
		std::vector< ci::Vec3f >::iterator n = normals.begin();
		for(r = 0; r < rings; r++) for(s = 0; s < sectors; s++) {
			float const y = sin( -M_PI_2 + M_PI * r * R );
			float const x = cos(2*M_PI * s * S) * sin( M_PI * r * R );
			float const z = sin(2*M_PI * s * S) * sin( M_PI * r * R );
			
			*v++ = ci::Vec3f( x * radius, y * radius, z * radius );
			*n++ = ci::Vec3f( x, y, z );
		}
		
		indices.resize(rings * sectors * 6);
		std::vector<uint32_t>::iterator i = indices.begin();
		for(r = 0; r < rings-1; r++) for(s = 0; s < sectors-1; s++) {
			*i++ = r * sectors + s;
			*i++ = r * sectors + (s+1);
			*i++ = (r+1) * sectors + (s+1);
			*i++ = (r+1) * sectors + s;
		}
		
		mSphericalTrimesh->appendVertices( vertices.data(), vertices.size() );
		mSphericalTrimesh->appendIndices( indices.data(), indices.size() );
		mSphericalTrimesh->recalculateNormals();
	}
	
private:
	// PRIVATE MEMBERS
	
	ci::gl::VboRef	mSphericalVbo, mSphericalNormalVbo, mSphericalElementVbo;
	ci::gl::VaoRef	mSphericalVao;
	ci::TriMeshRef	mSphericalTrimesh;
	GLuint			mTexBuffer;
};
}