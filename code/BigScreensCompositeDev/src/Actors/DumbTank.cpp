//
//  DumbTank.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/10/13.
//
//

#include "DumbTank.h"
#include "Utilities.hpp"

using namespace bigscreens;
using namespace ci;
using namespace ci::app;
using namespace std;

void DumbTank::load()
{
    // TEST
    mBarrelAngleDeg = 0;//-90.0f;
    mHeadRotationDeg = 0.0f;

    loadShader();
    loadModels();
}

void DumbTank::loadShader()
{
    gl::GlslProg::Format mRenderFormat;
    mRenderFormat.vertex( ci::app::loadResource("renderDumbTank.vert") )
	.geometry( ci::app::loadResource("renderDumbTank.geom") )
    .fragment( ci::app::loadResource("renderDumbTank.frag") );
    mRenderTankShader = gl::GlslProg::create( mRenderFormat );
}

void DumbTank::loadModels()
{
	DataSourceRef file = loadResource( "tank.obj" );
    ObjLoader loader( file );
    mMesh = TriMesh::create( loader );
    
    mFeedbackPositionVbo[0] = gl::Vbo::create(GL_ARRAY_BUFFER, mMesh->getNumVertices() * sizeof( Vec3f ),
						   mMesh->getVertices<3>(), GL_DYNAMIC_COPY );
    mFeedbackPositionVbo[1] = gl::Vbo::create( GL_ARRAY_BUFFER, mMesh->getNumVertices() * sizeof( Vec3f ), GL_DYNAMIC_COPY );
	
	mInitialTankPositionVbo = gl::Vbo::create( GL_ARRAY_BUFFER, mMesh->getNumVertices() * sizeof( Vec3f ), mMesh->getVertices<3>(), GL_STATIC_DRAW );
	
	mTankPositionNormals = gl::Vbo::create( GL_ARRAY_BUFFER, mMesh->getNumVertices() * sizeof( Vec3f ), mMesh->getNormals().data(), GL_STATIC_DRAW );
	
	mVao[0] = gl::Vao::create();
	mVao[1] = gl::Vao::create();
	
    mVao[0]->bind();
	
	mFeedbackPositionVbo[0]->bind();
	
    gl::enableVertexAttribArray( 0 );
    gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	
	mInitialTankPositionVbo->bind();
	
	gl::enableVertexAttribArray( 1 );
	gl::vertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	
	mTankPositionNormals->bind();
	
	gl::enableVertexAttribArray( 2 );
	gl::vertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	
	mVao[1]->bind();
	
	mFeedbackPositionVbo[1]->bind();
	
	gl::enableVertexAttribArray( 0 );
	gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	
	mInitialTankPositionVbo->bind();
	
	gl::enableVertexAttribArray( 1 );
	gl::vertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	
	mTankPositionNormals->bind();
	
	gl::enableVertexAttribArray( 2 );
	gl::vertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    
    mElementVbo = gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER,
                                  mMesh->getNumIndices() * 4,
                                  mMesh->getIndices().data(), GL_STATIC_DRAW );
	
	glGenTransformFeedbacks( 2, mTFOs );
	
	glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, mTFOs[0] );
	glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mFeedbackPositionVbo[0]->getId() );
	
	glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, mTFOs[1] );
	glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mFeedbackPositionVbo[1]->getId() );
    
	glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, 0 );
	
	
}

void DumbTank::update(long progressCounter)
{
    mHeadRotationDeg += 0.5;
    
    FiringTank::update(progressCounter);
}

ci::gl::VaoRef DumbTank::getVao()
{
    return mVao[0];
}

ci::gl::VboRef DumbTank::getElementVbo()
{
    return mElementVbo;
}

ci::TriMeshRef DumbTank::getMesh()
{
    return mMesh;
}

void DumbTank::draw( const int zFactor, const Vec3f & seperationPoint )
{
    gl::enableAdditiveBlending();
    
    mRenderTankShader->bind();
    
    // NOTE: There are so many lines, we throttle the alpha to 0.25 max
	mRenderTankShader->uniform( "projection", ci::gl::getProjection() );
	mRenderTankShader->uniform( "modelView", ci::gl::getModelView() );
	mRenderTankShader->uniform( "zFactor", (float)zFactor );
	mRenderTankShader->uniform( "seperationPoint", seperationPoint );
    
    mVao[0]->bind();
    mElementVbo->bind();
    
    int numIndices = mMesh->getNumIndices();
    
    gl::drawElements(GL_TRIANGLES,
                     numIndices,
                     GL_UNSIGNED_INT,
                     0);
    
    mElementVbo->unbind();
    mVao[0]->unbind();

    mRenderTankShader->unbind();
	gl::disableAlphaBlending();
}
