//
//  TankContent.cpp
//  GridMaker
//
//  Created by William Lindmeier on 10/8/13.
//
//

#include "TankContent.h"
#include "cinder/ObjLoader.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include "cinder/Camera.h"
#include "Utilities.hpp"

using namespace ci;
using namespace ci::app;
using namespace bigscreens;

namespace bigscreens {

void TankContent::load(const std::string & objFilename)
{
	gl::GlslProg::Format mFormat;
	mFormat.vertex( loadAsset( SharedShaderPath() + "basic.vert" ) )
		.fragment( loadAsset( SharedShaderPath() + "basic.frag" ) );
	mGlsl = gl::GlslProg::create( mFormat );
	mGlsl->bind();
    DataSourceRef file = loadAsset( objFilename ); //"T72.obj"
    ObjLoader loader( file );
	mMesh = TriMesh::create( loader );
//	loader.ge
	std::cout << " my mesh is " << mMesh->getNumVertices() << " large and there are " << mMesh->getNumIndices() << " indices"<< std::endl;
	mVao = gl::Vao::create();
	mVao->bind();
	
	mVbo = gl::Vbo::create( GL_ARRAY_BUFFER, 3 * mMesh->getNumVertices() * sizeof(float), mMesh->getVertices<3>(), GL_STATIC_DRAW );
	mVbo->bind();
	GLint pos = mGlsl->getAttribLocation( "position" );
	gl::enableVertexAttribArray( pos );
	gl::vertexAttribPointer( pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	mElementVbo = gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, mMesh->getNumIndices() * sizeof(unsigned int), mMesh->getIndices().data(), GL_STATIC_DRAW );
	mElementVbo->bind();
	
	mElementVbo->unbind();
	mVbo->unbind();
	mVao->unbind();
	mGlsl->unbind();

    mRotation.setToIdentity();
	mCam.setPerspective( 45.0f, (float)getWindowWidth() / getWindowHeight(), .01, 4000 );
	mCam.lookAt( Vec3f( 0, 0, 1000 ), Vec3f( 0, 0, 0 ) );
}

void TankContent::reset()
{
    mRotation.setToIdentity();
}

void TankContent::update()
{
    mRotation.rotate( Vec3f( 0, 1, 0 ), 0.006f );
}

void TankContent::render()
{
	// clear out both of the attachments of the FBO with black
	gl::clear( ColorAf( 0.0f, 0.0f, 0.0f, 1.0f ) );

	mGlsl->bind();
	mVao->bind();
	mElementVbo->bind();
	
	gl::pushMatrices();
	gl::setMatrices( mCam );
	gl::multModelView( Matrix44f::createTranslation( Vec3f( 0, -100, 0 )));
    gl::multModelView( mRotation );
    mGlsl->uniform( "projection", gl::getProjection() );
	mGlsl->uniform( "modelView", gl::getModelView() );
	
	gl::drawElements( GL_LINES, mMesh->getNumIndices(), GL_UNSIGNED_INT, 0 );
    gl::popMatrices();
	
	mVao->unbind();
	mGlsl->unbind();
	mElementVbo->unbind();
	
}
	
}
