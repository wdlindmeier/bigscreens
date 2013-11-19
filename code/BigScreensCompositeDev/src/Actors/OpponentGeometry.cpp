//
//  OpponentGeometry.cpp
//  BigScreensComposite
//
//  Created by Ryan Bartley on 11/8/13.
//
//

#include "OpponentGeometry.h"
#include "Utilities.hpp"

namespace bigscreens {
	
// ---------------------------------------------------------------------------------
// DYNAMIC OPPONENT GEOMETRY
DynamicOpponent::DynamicOpponent()
	: drawBuf( 1 ), mPyramid( new SphericalGeometry(false) ),
		mSphere( new SphericalGeometry(true) ), mTrimesh( mPyramid->getTrimesh() )
{
	loadShaders();
	loadBuffers();
	loadTexture();
	createRandomMap();
}
	
void DynamicOpponent::update( float percentage, float time )
{
	GLuint query;
	glGenQueries(1, &query);
	
	drawBuf = 1 - drawBuf;
	
	mUpdateOppDynamicGlsl->bind();
	
	mPyramid->bindTexBuffer( GL_TEXTURE1 );
	mSphere->bindTexBuffer( GL_TEXTURE2 );

	glActiveTexture( GL_TEXTURE3 );
	glBindTexture( GL_TEXTURE_2D, mRandomTexture->getId() );
	
	mUpdateOppDynamicGlsl->uniform( "pyramidalTex", 1 );
	mUpdateOppDynamicGlsl->uniform( "sphericalTex", 2 );
	mUpdateOppDynamicGlsl->uniform( "randomMap", 3 );
	mUpdateOppDynamicGlsl->uniform( "percentage", percentage );
	mUpdateOppDynamicGlsl->uniform( "frameNum", (int)ci::app::getElapsedFrames() );
	
	glEnable( GL_RASTERIZER_DISCARD );
	
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
	
	glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, mTFOs[1-drawBuf] );
	
	glBeginTransformFeedback(GL_POINTS);
	mVao[drawBuf]->bind();
	ci::gl::drawArrays( GL_POINTS, 0, mPyramid->getTrimesh()->getNumVertices() );
	mVao[drawBuf]->unbind();
	glEndTransformFeedback();
	
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	
	GLuint vertices;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &vertices);
	
	glDisable(GL_RASTERIZER_DISCARD);


	glActiveTexture( GL_TEXTURE3 );
	glBindTexture( GL_TEXTURE_2D, mRandomTexture->getId() );
	mSphere->unbindTexBuffer( GL_TEXTURE2 );
	mPyramid->unbindTexBuffer( GL_TEXTURE1 );
	mUpdateOppDynamicGlsl->unbind();
}

void DynamicOpponent::draw( const ci::Vec3f & cameraView )
{
	mRenderOppDynamicGlsl->bind();
	
	mRenderOppDynamicGlsl->uniform( "projection", ci::gl::getProjection() );
	mRenderOppDynamicGlsl->uniform( "modelView", ci::gl::getModelView() );
	mRenderOppDynamicGlsl->uniform( "lightPosition", cameraView );
	
	mVao[1-drawBuf]->bind();
	mElementVbo->bind();
	
	ci::gl::drawElements( GL_LINES_ADJACENCY, mTrimesh->getNumIndices(), GL_UNSIGNED_INT, 0 );
	
	mElementVbo->unbind();
	mVao[1-drawBuf]->unbind();
	mPyramid->draw();
	
	mRenderOppDynamicGlsl->unbind();
	
}
	
void DynamicOpponent::loadBuffers()
{
	mPositionVbo[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mTrimesh->getNumVertices() * sizeof(ci::Vec3f), mTrimesh->getVertices<3>(), GL_DYNAMIC_COPY );
	mPositionVbo[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mTrimesh->getNumVertices() * sizeof(ci::Vec3f), GL_DYNAMIC_COPY );
	
	mElementVbo = ci::gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, mTrimesh->getNumIndices() * sizeof(uint32_t), mTrimesh->getIndices().data(), GL_STATIC_DRAW );
	
	mVao[0] = ci::gl::Vao::create();
	mVao[1] = ci::gl::Vao::create();
	
	mVao[0]->bind();
	
	mPositionVbo[0]->bind();
	ci::gl::enableVertexAttribArray(0);
	ci::gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	
	mVao[1]->bind();
	
	mPositionVbo[1]->bind();
	ci::gl::enableVertexAttribArray(0);
	ci::gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	
	glGenTransformFeedbacks(2, mTFOs);
	
	glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, mTFOs[0] );
	glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mPositionVbo[0]->getId() );

	glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, mTFOs[1] );
	glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mPositionVbo[1]->getId() );

	glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, 0 );
}
	
void DynamicOpponent::loadShaders()
{
	const char * varyings[] = {
		"outPosition"
	};
	
	ci::gl::GlslProg::Format mUpdateOppDynamicGlslFormat;
	mUpdateOppDynamicGlslFormat.vertex( /*LoadShader("oppDynamic.vert")*/ ci::app::loadResource( "updateOppDynamic.vert" ) )
	.transformFeedback().feedbackVaryings( varyings, 1 )
	.feedbackFormat( GL_SEPARATE_ATTRIBS );
	mUpdateOppDynamicGlsl = ci::gl::GlslProg::create( mUpdateOppDynamicGlslFormat );

	
	ci::gl::GlslProg::Format mRenderOppDynamicGlslFormat;
	mRenderOppDynamicGlslFormat.vertex( /*LoadShader("oppDynamic.vert")*/ ci::app::loadResource( "renderOppDynamic.vert" ))
	.geometry( /*LoadShader("oppDynamic.geom")*/ ci::app::loadResource( "renderOppDynamic.geom" ) )
	.fragment( /*LoadShader("oppDynamic.frag" )*/ ci::app::loadResource( "renderOppDynamic.frag" ) );
	mRenderOppDynamicGlsl = ci::gl::GlslProg::create( mRenderOppDynamicGlslFormat );
	
}
	
void DynamicOpponent::createRandomMap()
{
	ci::Surface32f mSurface(400, 400, false);
	auto iter = mSurface.getIter();
	while(iter.line()) {
		while(iter.pixel()) {
			iter.r() = ci::randFloat( 0.0f, 1.0f );
			iter.g() = ci::randFloat( 0.0f, 1.0f );
			iter.b() = ci::randFloat( 0.0f, 1.0f );
		}
	}
	
	mRandomTexture = ci::gl::Texture::create( mSurface );
}
	
void DynamicOpponent::loadTexture()
{
	mNoiseTexture = ci::gl::Texture::create( ci::loadImage( ci::app::loadResource( "noise_map.png"  ) ) );
}
	
// ----------------------------------------------------------------------------------
// SPHERICAL OPPONENT GEOMETRY

void SphericalGeometry::draw() {
	mSphericalVao->bind();
	mSphericalElementVbo->bind();
	
	ci::gl::drawElements( GL_LINES_ADJACENCY, mSphericalTrimesh->getNumIndices(), GL_UNSIGNED_INT, 0);

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
	
	mSphericalElementVbo = ci::gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, mSphericalTrimesh->getNumIndices() * sizeof( uint32_t ), mSphericalTrimesh->getIndices().data(), GL_STATIC_DRAW );
	mSphericalVao->unbind();
	
	glGenTextures( 1, &mTexBuffer );
	glBindTexture( GL_TEXTURE_BUFFER, mTexBuffer );
	glTexBuffer( GL_TEXTURE_BUFFER, GL_RGB32F, mSphericalVbo->getId() );
}
	
void SphericalGeometry::calcGeometry( float radius, unsigned int rings, unsigned int sectors, bool sphere )
{
	float const R = 1.0f/(float)(rings-1);
	float const S = 1.0f/(float)(sectors-1);
	float r, s;
	float pyramidStep = 0.0f;
	int sInt = 0;
	
	std::vector< ci::Vec3f > vertices;
	std::vector< ci::Vec3f > normals;
	std::vector< uint32_t > indices;
	
	vertices.resize(rings * sectors);
	normals.resize(rings * sectors);
	std::vector< ci::Vec3f >::iterator v = vertices.begin();
	std::vector< ci::Vec3f >::iterator n = normals.begin();
	
	if( !sphere ) {
		for(r = 0; r < rings; r++) {
			float x = 0.0f;
			float z = 0.0f;
			for(s = 0; s < sectors; s++) {
				sInt = s;
				
				float const y = sin( -M_PI_2 + M_PI * r * R );
				if( sInt % 5 == 0) {
					x = cos(2*M_PI * s * S) * sin( M_PI * r * R ) * pyramidStep;
					z = sin(2*M_PI * s * S) * sin( M_PI * r * R ) * pyramidStep;
				}
				
				*v++ = ci::Vec3f( x * radius, y * radius, z * radius );
				*n++ = ci::Vec3f( x, y, z );
			}
			if( r < 10 )
				pyramidStep += 0.05;
			else
				pyramidStep -= 0.05;
		}
	}
	else {
		for(r = 0; r < rings; r++) for(s = 0; s < sectors; s++) {
			float const y = sin( -M_PI_2 + M_PI * r * R );
			float const x = cos(2*M_PI * s * S) * sin( M_PI * r * R );
			float const z = sin(2*M_PI * s * S) * sin( M_PI * r * R );
			
			*v++ = ci::Vec3f( x * radius, y * radius, z * radius );
			*n++ = ci::Vec3f( x, y, z );
		}
	}
	indices.resize(rings * sectors * 4);
	std::vector<uint32_t>::iterator i = indices.begin();
	for(r = 0; r < rings-1; r++) { for(s = 0; s < sectors-1; s++) {
		*i++ = r * sectors + s;
		*i++ = r * sectors + (s+1);
		*i++ = (r+1) * sectors + (s+1);
		*i++ = (r+1) * sectors + s;
	}}
	
	mSphericalTrimesh->appendVertices( vertices.data(), vertices.size() );
	mSphericalTrimesh->appendIndices( indices.data(), indices.size() );
	mSphericalTrimesh->recalculateNormals();
}
	
// ---------------------------------------------------------------------------------
// PYRAMIDAL OPPONENT GEOMETRY
	
void MinionGeometry::draw( const ci::Vec3f & lightPosition, const ci::ColorA & minionColor  )
{
	// LightPosition is the camera's position in the world
	ci::gl::enableDepthWrite();
	ci::gl::enableDepthRead();
	
	mPyramidalVao->bind();
	mPyramidalElementVbo->bind();
	
	mGlsl->bind();
	mGlsl->uniform( "projection", ci::gl::getProjection() );
	mGlsl->uniform( "modelView", ci::gl::getModelView() );
	mGlsl->uniform( "normalMatrix", ci::gl::calcNormalMatrix() );
	mGlsl->uniform( "lightPosition", lightPosition);
	mGlsl->uniform( "mColor", minionColor );
	
	ci::gl::drawElements( GL_TRIANGLES, mPyramidalTrimesh->getNumIndices(), GL_UNSIGNED_INT, 0 );
	
	mGlsl->unbind();
	
	mPyramidalElementVbo->unbind();
	mPyramidalVao->unbind();
	
	ci::gl::disableDepthWrite();
	ci::gl::disableDepthRead();
}
	
void MinionGeometry::loadBuffers()
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
	
void MinionGeometry::loadShaders()
{
    mGlsl = ci::gl::GlslProg::create( ci::gl::GlslProg::Format()
                                     .vertex( ci::app::loadResource( "minion.vert" ) )
                                     .fragment( ci::app::loadResource( "minion.frag" ) ) );
}
	
void MinionGeometry::calcGeometry()
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