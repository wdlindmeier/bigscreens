//
//  PyramidalGeometry.h
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
	
typedef std::shared_ptr<class PyramidalGeometry> PyramidalGeometryRef;

class PyramidalGeometry {
public:
	PyramidalGeometry()
	{
		ci::TriMesh::Format mPyramidalTrimeshFormat;
		mPyramidalTrimeshFormat.vertices(3).normals();
		mPyramidalTrimesh = ci::TriMesh::create( mPyramidalTrimeshFormat );
		
		calcGeometry();
		loadBuffers();
	}
	~PyramidalGeometry(){}
	
	void draw() {
		mPyramidalVao->bind();
		mPyramidalElementVbo->bind();
		
		ci::gl::drawElements( GL_TRIANGLES, mPyramidalTrimesh->getNumIndices(), GL_UNSIGNED_INT, 0 );
		
		mPyramidalElementVbo->unbind();
		mPyramidalVao->unbind();
	}
	
private:
	
	// SETUP FUNCTIONS
	void loadBuffers()
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
	}
	void calcGeometry()
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
	
private:
	// PRIVATE MEMBERS
	
	ci::gl::VaoRef mPyramidalVao;
	ci::gl::VboRef mPyramidalVbo, mPyramidalNormalVbo, mPyramidalElementVbo;
	ci::TriMeshRef mPyramidalTrimesh;
};
	
}
