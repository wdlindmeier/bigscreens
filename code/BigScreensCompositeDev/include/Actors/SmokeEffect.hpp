//
//  SmokeEffect.hpp
//  Opponent
//
//  Created by Ryan Bartley on 11/3/13.
//
//

#pragma once

#include <iostream>
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "cinder/gl/Texture.h"
#include "Utilities.hpp"


namespace bigscreens {
	
const int nParticles = 4000;
	
typedef std::shared_ptr<class SmokeEffect> SmokeEffectRef;
	
class SmokeEffect {
public:
	SmokeEffect()
	: drawBuf(1), time(0), deltaT(0)
	{
		ci::TriMesh::Format mTrimeshFormat;
		mTrimeshFormat.positions(3).normals();
		mTrimesh = ci::TriMesh::create( mTrimeshFormat );
		
		initBuffers();
		loadShaders();
		loadTexture();
		
		mCam.setPerspective( 60.0f, ci::app::getWindowAspectRatio(), .1, 1000);
		mCam.lookAt( ci::Vec3f( 0, 0, 10 ), ci::Vec3f( 0, 0, 0 ) );
	}
	~SmokeEffect(){}
	
	void draw(float zDepth)
	{
		update();
		render(zDepth);
	}
	
	void update()
	{
		drawBuf = 1 - drawBuf;
		deltaT = ci::app::getElapsedSeconds() - time;
		time = ci::app::getElapsedSeconds();
		mOpponentParticlesGlsl->bind();
		
		// UPDATE
		glUniformSubroutinesuiv( GL_VERTEX_SHADER, 1, &updateSub );
		
		mOpponentParticlesGlsl->uniform( "Time", time );
		mOpponentParticlesGlsl->uniform( "H", deltaT );
		
		ci::gl::enable( GL_RASTERIZER_DISCARD );
		
		glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, mTFOs[1-drawBuf] );
		
		glBeginTransformFeedback(GL_POINTS);
		mPVao[drawBuf]->bind();
		ci::gl::drawArrays( GL_POINTS, 0, nParticles );
		glEndTransformFeedback();
		
		ci::gl::disable( GL_RASTERIZER_DISCARD );
	}
	
	void render( float zDepth )
	{
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &renderSub);
		//		mSmokeTexture->bind();
		
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, mSmokeTexture->getId() );
		
		mPVao[1-drawBuf]->bind();
		
		// TODO: Make the scaling parabolic
		float maxParticleSize = ci::lmap(zDepth, 40.0f, 0.1f, 1.0f, 64.0f );
		float scale = ci::lmap( zDepth, 40.0f, 0.1f, 0.0f, 0.4f );
		scale = ( scale > 0.1f ) ? scale : 0.1f;
		
		ci::gl::pushMatrices();
//		ci::gl::multProjection( ci::Matrix44f::createTranslation( ci::Vec3f( 0, 0 ,  ) ) );
		ci::gl::multModelView( ci::Matrix44f::createScale( ci::Vec3f( scale, scale, scale ) ) );
		
		ci::gl::enable( GL_PROGRAM_POINT_SIZE );
		
		ci::gl::enableAlphaBlending();
		
		mOpponentParticlesGlsl->uniform( "projection", ci::gl::getProjection() );
		mOpponentParticlesGlsl->uniform( "modelView", ci::gl::getModelView() );
		mOpponentParticlesGlsl->uniform( "MinParticleSize", 1.0f  );
		mOpponentParticlesGlsl->uniform( "MaxParticleSize", maxParticleSize > 10.0f ? maxParticleSize : 10.0f );
		mOpponentParticlesGlsl->uniform( "ParticleLifetime", 3.0f );
		
//		std::cout << maxParticleSize << " " << zDepth << " " << scale << std::endl;
		
		glDrawArrays( GL_POINTS, 0, nParticles);
		
		ci::gl::disable( GL_PROGRAM_POINT_SIZE );
		
		ci::gl::popMatrices();
		
		ci::gl::disableAlphaBlending();
		
		mPVao[1-drawBuf]->unbind();
		//		mSmokeTexture->unbind();
		glBindTexture( GL_TEXTURE_2D, 0 );
		mOpponentParticlesGlsl->unbind();
		
	}
	
	
private:
	
	void initBuffers()
	{
		glGenQueries(1, &query);
		
		// BUFFER FIRST POSITION DATA
		ci::Vec3f * data = new ci::Vec3f[nParticles];
		for( int i = 0; i < nParticles; i++ ) mTrimesh->appendVertex( ci::Vec3f( 0.0f, 0.0f, 0.0f ) );
		mPPositions[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mTrimesh->getNumVertices() * sizeof(ci::Vec3f), mTrimesh->getVertices<3>(), GL_DYNAMIC_COPY );
		mPPositions[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mTrimesh->getNumVertices() * sizeof(ci::Vec3f), GL_DYNAMIC_COPY );
		
		// BUFFER FIRST VELOCITY DATA
		ci::Vec3f v(0.0f, 0.0f, 0.0f);
		float velocity, theta, phi;
		for( int i = 0; i < nParticles; i++ ) {
			theta = mix( 0.0f, (float)pi / 6.0f, ci::randFloat() );
			phi = mix( 0.0f, (float)(2 * pi), ci::randFloat() );
			
//			v.x = sinf(i) * cosf(i);
//			v.y = cosf(phi) * sinf(i);
//			v.z = sinf(theta);
			v.x = cos( 2*M_PI * i * (i/nParticles) ) * sin( M_PI + i );
			v.y = sin( -M_PI_2 + M_PI + i );
			v.z = sin( 2* M_PI * i * (i/nParticles) ) * sin( M_PI + i );
			
			velocity = mix( 0.0f, 1.5f, ci::randFloat() );
			v = v.normalized() * velocity;
			mTrimesh->appendNormal( v );
		}
		mPVelocities[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mTrimesh->getNormals().size() * sizeof(ci::Vec3f), mTrimesh->getNormals().data(), GL_DYNAMIC_COPY );
		mPVelocities[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mTrimesh->getNormals().size() * sizeof(ci::Vec3f), GL_DYNAMIC_COPY );
		mPInitVelocity = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mTrimesh->getNormals().size() * sizeof(ci::Vec3f), mTrimesh->getNormals().data(), GL_STATIC_DRAW );
		
		delete [] data;
		
		// BUFFER START TIMES
		GLfloat * timeData = new GLfloat[nParticles];
		float time = 0.0f;
		float rate = 0.001f;
		for( int i = 0; i < nParticles; i++ ) {
			timeData[i] = time;
			time += rate;
		}
		mPStartTimes[0] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, nParticles * sizeof( float ), timeData, GL_DYNAMIC_COPY );
		mPStartTimes[1] = ci::gl::Vbo::create( GL_ARRAY_BUFFER, nParticles * sizeof( float ), GL_DYNAMIC_COPY );
		
		delete [] timeData;
		
		mPVao[0] = ci::gl::Vao::create();
		mPVao[1] = ci::gl::Vao::create();
		
		// INITIALIZE THE FIRST VAO
		mPVao[0]->bind();
		mPPositions[0]->bind();
		ci::gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
		ci::gl::enableVertexAttribArray( 0 );
		
		mPVelocities[0]->bind();
		ci::gl::vertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );
		ci::gl::enableVertexAttribArray( 1 );
		
		mPStartTimes[0]->bind();
		ci::gl::vertexAttribPointer( 2, 1, GL_FLOAT, GL_FALSE, 0, 0 );
		ci::gl::enableVertexAttribArray( 2 );
		
		mPInitVelocity->bind();
		ci::gl::vertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, 0, 0 );
		ci::gl::enableVertexAttribArray( 3 );
		
		// INITIALIZE THE SECOND VAO
		mPVao[1]->bind();
		mPPositions[1]->bind();
		ci::gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
		ci::gl::enableVertexAttribArray( 0 );
		
		mPVelocities[1]->bind();
		ci::gl::vertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );
		ci::gl::enableVertexAttribArray( 1 );
		
		mPStartTimes[1]->bind();
		ci::gl::vertexAttribPointer( 2, 1, GL_FLOAT, GL_FALSE, 0, 0 );
		ci::gl::enableVertexAttribArray( 2 );
		
		mPInitVelocity->bind();
		ci::gl::vertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, 0, 0 );
		ci::gl::enableVertexAttribArray( 3 );
		
		// INITIALIZE THE TRANSFORM FEEDBACK OBJECTS
		glGenTransformFeedbacks( 2, mTFOs );
		
		glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, mTFOs[0] );
		glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mPPositions[0]->getId() );
		glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 1, mPVelocities[0]->getId() );
		glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 2, mPStartTimes[0]->getId() );
		
		glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, mTFOs[1] );
		glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mPPositions[1]->getId() );
		glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 1, mPVelocities[1]->getId() );
		glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 2, mPStartTimes[1]->getId() );
		
		glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, 0 );
	}
	
	void loadTexture()
	{
		ci::gl::Texture::Format mTextureFormat;
		mTextureFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR_MIPMAP_LINEAR ).mipMap().internalFormat( GL_RGBA );
		mSmokeTexture = ci::gl::Texture::create( loadImage( ci::app::loadAsset( "smoke_blur.png" ) ), mTextureFormat );
	}
	
	void loadShaders()
	{
		try {
			const char * outputNames[] = {
				"Position",
				"Velocity",
				"StartTime"
			};
			
			ci::gl::GlslProg::Format mParticleGlslFormat;
			mParticleGlslFormat.vertex( ci::app::loadAsset( "oppSmoke.vert" /*SharedShaderAssetPath("oppSmoke.vert", !IS_IAC)*/ ) )
			.fragment( ci::app::loadAsset( "oppSmoke.frag" /*SharedShaderAssetPath("oppSmoke.geom", !IS_IAC)*/ ) )
			.transformFeedback().feedbackFormat( GL_SEPARATE_ATTRIBS )
			.feedbackVaryings( outputNames, 3 );
			
			mOpponentParticlesGlsl = ci::gl::GlslProg::create( mParticleGlslFormat );
		} catch ( ci::gl::GlslProgCompileExc ex ) {
			std::cout << "PARTICLE UPDATE GLSL ERROR: " << ex.what() << std::endl;
		}
		
		renderSub = glGetSubroutineIndex( mOpponentParticlesGlsl->getHandle(), GL_VERTEX_SHADER, "render");
		updateSub = glGetSubroutineIndex( mOpponentParticlesGlsl->getHandle(), GL_VERTEX_SHADER, "update");
		
		mOpponentParticlesGlsl->uniform( "ParticleTex", 0 );
		
		mOpponentParticlesGlsl->uniform( "Accel", ci::Vec3f( 0.3f, 0.3f, 0.3f ) );
		mOpponentParticlesGlsl->uniform( "ParticleTex", 0 );
		
	}
	
	float mix( float x, float y, float a )
	{
		return x * ( 1 - a ) + y * a;
	}
	
private:
	ci::gl::VaoRef		mPVao[2];
	GLuint				mTFOs[2];
	ci::gl::VboRef		mPPositions[2], mPVelocities[2], mPStartTimes[2], mPInitVelocity;
	ci::gl::GlslProgRef	mOpponentParticlesGlsl, mParticleRenderGlsl;
	ci::gl::TextureRef	mSmokeTexture;
	ci::CameraPersp		mCam;
	float				time;
	float				deltaT;
	GLuint				renderSub, updateSub, query, drawBuf;
	ci::TriMeshRef		mTrimesh;
};
	
}