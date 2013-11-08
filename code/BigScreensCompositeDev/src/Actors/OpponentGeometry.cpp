//
//  OpponentGeometry.cpp
//  BigScreensComposite
//
//  Created by Ryan Bartley on 11/8/13.
//
//

#pragma once

#include "OpponentGeometry.h"
#include "Utilities.hpp"

namespace bigscreens {
	
// ---------------------------------------------------------------------------------
// DYNAMIC OPPONENT GEOMETRY
DynamicOpponent::DynamicOpponent() {
	
	mPyramid = PyramidalGeometryRef( new PyramidalGeometry() );
	mSphere = SphericalGeometryRef( new SphericalGeometry() );
	
	ci::TriMesh::Format mFormat;
	mFormat.positions(3);
	
	mTrimesh = ci::TriMesh::create( mFormat );
	
	loadShaders();
	loadGeometry();
	loadBuffers();
}
	
void DynamicOpponent::update()
{
	
}

void DynamicOpponent::draw()
{
	
}
	
void DynamicOpponent::loadBuffers()
{
	
}
	
void DynamicOpponent::loadGeometry()
{
	for( int i = 0; i < 1200; i++ ) {
		mTrimesh->appendVertex( ci::Vec3i( 0.0f, 0.0f, 0.0f ) );
	}
	
	//		mTrimesh->appendIndices( , );
}
	
void DynamicOpponent::loadShaders()
{
	const char * varyings[] = {
		"position"
	};
	
	ci::gl::GlslProg::Format mGlslFormat;
	mGlslFormat.vertex( ci::app::loadAsset( /*"oppDynamic.vert"*/ SharedShaderAssetPath("oppDynamic.vert", !IS_IAC) ) )
	.geometry( ci::app::loadAsset( /*"oppDynamic.geom"*/ SharedShaderAssetPath("oppDynamic.geom", !IS_IAC) ) )
	.fragment( ci::app::loadAsset( /*"oppDynamic.geom"*/ SharedShaderAssetPath("oppDynamic.frag", !IS_IAC) ) )
	.transformFeedback().feedbackVaryings( varyings, 1 )
	.feedbackFormat( GL_SEPARATE_ATTRIBS );
	
}
	
// ----------------------------------------------------------------------------------
// SPHERICAL OPPONENT GEOMETRY

void SphericalGeometry::draw() {
	mSphericalVao->bind();
	mSphericalElementVbo->bind();
	
	//		ci::gl::drawElements( GL_TRIANGLES, mSphericalTrimesh->getNumIndices(), GL_UNSIGNED_INT, 0);
	ci::gl::drawArrays( GL_TRIANGLES, 0, mSphericalTrimesh->getNumVertices() );
	mSphericalElementVbo->unbind();
	mSphericalVao->unbind();
}
	
void SphericalGeometry::loadBuffers()
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
	
void SphericalGeometry::calcGeometry( float radius, unsigned int rings, unsigned int sectors )
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
	
// ---------------------------------------------------------------------------------
// PYRAMIDAL OPPONENT GEOMETRY
	
void PyramidalGeometry::draw() {
	mPyramidalVao->bind();
	mPyramidalElementVbo->bind();
	
	ci::gl::drawElements( GL_TRIANGLES, mPyramidalTrimesh->getNumIndices(), GL_UNSIGNED_INT, 0 );
	
	mPyramidalElementVbo->unbind();
	mPyramidalVao->unbind();
}
	
void PyramidalGeometry::loadBuffers()
{
	mPyramidalVao = ci::gl::Vao::create();
	mPyramidalVao->bind();
	
	mPyramidalVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mPyramidalTrimesh->getNumVertices() * sizeof( ci::Vec3f ), mPyramidalTrimesh->getVertices<3>(), GL_STATIC_DRAW );
	mPyramidalVbo->bind();
	ci::gl::enableVertexAttribArray( 0 );
	ci::gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	
	mPyramidalNormalVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mPyramidalTrimesh->getNumVertices() * sizeof( ci::Vec3f ), mPyramidalTrimesh->getNormals().data(), GL_STATIC_DRAW );
	mPyramidalNormalVbo->bind();
	ci::gl::enableVertexAttribArray( 1 );
	ci::gl::vertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	
	mPyramidalElementVbo = ci::gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, mPyramidalTrimesh->getNumIndices() * sizeof( uint32_t ), mPyramidalTrimesh->getIndices().data(), GL_STATIC_DRAW );
	
	mPyramidalVao->unbind();
	
	glGenTextures( 1, &mTexBuffer );
	glBindTexture( GL_TEXTURE_BUFFER, mTexBuffer );
	glTexBuffer( GL_TEXTURE_BUFFER, GL_RGBA32F, mPyramidalVbo->getId() );
}
	
void PyramidalGeometry::calcGeometry()
{
	std::vector< ci::Vec3f > vertices;
	std::vector< ci::Vec3f > normals;
	std::vector< ci::uint32_t > indices;
	
	vertices.push_back( ci::Vec3f(   0,  1,   0 ) ); // 0 - Top of the Triangle;
	vertices.push_back( ci::Vec3f( -.5,  0,  .5 ) ); // 1 - Front Left of the Triangle;
	vertices.push_back( ci::Vec3f(  .5,  0,  .5 ) ); // 2 - Front Right of the Triangle;
	vertices.push_back( ci::Vec3f( -.5,  0, -.5 ) ); // 3 - Back Left of the Triangle;
	vertices.push_back( ci::Vec3f(  .5,  0, -.5 ) ); // 4 - Back Right of the Triangle;
	vertices.push_back( ci::Vec3f(   0, -1,   0 ) ); // 5 - Bottom of the Triangle;
	
	indices.push_back( 0 ); // top
	indices.push_back( 1 ); // front left
	indices.push_back( 2 ); // front right
	indices.push_back( 0 ); // top
	indices.push_back( 2 ); // front right
	indices.push_back( 4 ); // back  right
	indices.push_back( 0 ); // top
	indices.push_back( 4 ); // back right
	indices.push_back( 3 ); // back left
	indices.push_back( 0 ); // top
	indices.push_back( 3 ); // back left
	indices.push_back( 1 ); // front left
	indices.push_back( 5 ); // bottom
	indices.push_back( 2 ); // front right
	indices.push_back( 1 ); // front left
	indices.push_back( 5 ); // bottom
	indices.push_back( 4 ); // back right
	indices.push_back( 2 ); // front right
	indices.push_back( 5 ); // bottom
	indices.push_back( 3 ); // back left
	indices.push_back( 4 ); // back right
	indices.push_back( 5 ); // bottom
	indices.push_back( 1 ); // front left
	indices.push_back( 3 ); // back left
	
	mPyramidalTrimesh->appendVertices( vertices.data(), vertices.size() );
	mPyramidalTrimesh->appendIndices( indices.data(), indices.size() );
	mPyramidalTrimesh->recalculateNormals();
}
	
}