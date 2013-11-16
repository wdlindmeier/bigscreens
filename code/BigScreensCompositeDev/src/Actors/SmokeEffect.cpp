//
//  SmokeEffect.cpp
//  Opponent
//
//  Created by Ryan Bartley on 11/3/13.
//
//

#include "SmokeEffect.h"

namespace bigscreens {

SmokeEffect::SmokeEffect()
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
	
void SmokeEffect::update( const ci::Vec3f & accel, float currentTime )
{
	drawBuf = 1 - drawBuf;
	deltaT = ci::app::getElapsedSeconds() - time;
	time = ci::app::getElapsedSeconds();
	
	mUpdateOpponentParticlesGlsl->bind();
	
	// UPDATE
	
	mUpdateOpponentParticlesGlsl->uniform( "Time", time );
	mUpdateOpponentParticlesGlsl->uniform( "H", deltaT );
	mUpdateOpponentParticlesGlsl->uniform( "Accel", accel );
	mUpdateOpponentParticlesGlsl->uniform( "ParticleLifetime", 3.0f );
	
	ci::gl::enable( GL_RASTERIZER_DISCARD );
	
	glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, mTFOs[1-drawBuf] );
	
	glBeginTransformFeedback( GL_POINTS );
	mPVao[drawBuf]->bind();
	ci::gl::drawArrays( GL_POINTS, 0, nParticles );
	glEndTransformFeedback();
	
	ci::gl::disable( GL_RASTERIZER_DISCARD );
	mUpdateOpponentParticlesGlsl->unbind();
}

void SmokeEffect::draw( float zDepth )
{
	
	mRenderOpponentParticlesGlsl->bind();
	//		mSmokeTexture->bind();
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, mSmokeTexture->getId() );
//	mSmokeTexture->bind();
	
	mPVao[1-drawBuf]->bind();
	
	// TODO: Make the scaling parabolic
//	float maxParticleSize = ci::lmap(zDepth, 40.0f, 0.1f, 1.0f, 64.0f );
//	float scale = ci::lmap( zDepth, 40.0f, 0.1f, 0.0f, 0.4f );
//	scale = ( scale > 0.1f ) ? scale : 0.1f;
	
	ci::gl::pushMatrices();
	//		ci::gl::multProjection( ci::Matrix44f::createTranslation( ci::Vec3f( 0, 0 ,  ) ) );
//	ci::gl::multModelView( ci::Matrix44f::createScale( ci::Vec3f( scale, scale, scale ) ) );
	
	ci::gl::enable( GL_PROGRAM_POINT_SIZE );
	
	ci::gl::enableAlphaBlending();
	
	mRenderOpponentParticlesGlsl->uniform( "Time", time );
	mRenderOpponentParticlesGlsl->uniform( "projection", ci::gl::getProjection() );
	mRenderOpponentParticlesGlsl->uniform( "modelView", ci::gl::getModelView() );
	mRenderOpponentParticlesGlsl->uniform( "MinParticleSize", 1.0f  );
	mRenderOpponentParticlesGlsl->uniform( "MaxParticleSize", 64.0f );//maxParticleSize > 10.0f ? maxParticleSize : 10.0f );
	mRenderOpponentParticlesGlsl->uniform( "ParticleLifetime", 3.0f );
	
	
	//		std::cout << maxParticleSize << " " << zDepth << " " << scale << std::endl;
	
	glDrawArrays( GL_POINTS, 0, nParticles);
	
	ci::gl::disable( GL_PROGRAM_POINT_SIZE );
	
	ci::gl::popMatrices();
	
	ci::gl::disableAlphaBlending();
	
	mPVao[1-drawBuf]->unbind();
	glBindTexture( GL_TEXTURE_2D, 0 );
	
//	mSmokeTexture->unbind();
	mRenderOpponentParticlesGlsl->unbind();
	
}

void SmokeEffect::initBuffers()
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
		//			v.x = cos( 2*M_PI * i * (i/nParticles) ) * sin( M_PI + i );
		//			v.y = sin( -M_PI_2 + M_PI + i );
		//			v.z = sin( 2* M_PI * i * (i/nParticles) ) * sin( M_PI + i );
		
		float angle = ci::randFloat( 0.0f, pi*2.0f );// random(0,TWO_PI);
		v.z = ci::randFloat( -1.0f, 1.0 );
		v.x = sqrt(1-v.z*v.z)*cos(angle);
		v.y = sqrt(1-v.z*v.z)*sin(angle);
		
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

void SmokeEffect::loadTexture()
{
	ci::gl::Texture::Format mTextureFormat;
	mTextureFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR_MIPMAP_LINEAR ).mipMap().internalFormat( GL_RGBA );
	mSmokeTexture = ci::gl::Texture::create( loadImage( ci::app::loadResource( "smoke_blur.png" ) ), mTextureFormat );
}

void SmokeEffect::loadShaders()
{
	try {
		const char * outputNames[] = {
			"Position",
			"Velocity",
			"StartTime"
		};
		
		ci::gl::GlslProg::Format mUpdateParticleGlslFormat;
        // NOTE: See SharedShaderAssetPath to dynamically load
		mUpdateParticleGlslFormat.vertex( /*LoadShader("oppSmoke.vert")*/ ci::app::loadAsset( "updateOppSmoke.vert" ) )
		.transformFeedback().feedbackFormat( GL_SEPARATE_ATTRIBS )
		.feedbackVaryings( outputNames, 3 );
		
		mUpdateOpponentParticlesGlsl = ci::gl::GlslProg::create( mUpdateParticleGlslFormat );
	}
	catch ( ci::gl::GlslProgCompileExc ex ) {
		std::cout << "PARTICLE UPDATE GLSL ERROR: " << ex.what() << std::endl;
	}
	
	try {
		ci::gl::GlslProg::Format mRenderParticleGlslFormat;
		mRenderParticleGlslFormat.vertex( ci::app::loadAsset( "renderOppSmoke.vert" ) )
		.fragment( ci::app::loadAsset( "renderOppSmoke.frag" ) );
		
		mRenderOpponentParticlesGlsl = ci::gl::GlslProg::create( mRenderParticleGlslFormat );
	}
	catch( ci::gl::GlslProgCompileExc ex ) {
		std::cout << "PARTICLE RENDER GLSL ERROR: " << ex.what() << std::endl;
	}
	
	mRenderOpponentParticlesGlsl->uniform( "ParticleTex", 0 );
}

}