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
	SmokeEffect();
	~SmokeEffect(){}
	
	void render(float zDepth, const ci::Vec3f & accel )
	{
//		update(accel, );
//		draw(zDepth);
	}
	
	//void update( const ci::Vec3f & accel, float time );
    void update( const ci::Vec3f & accel, long numFramesRendered );
	
	void draw( float zDepth, long numFramesRendered );
    void resetRand();
	
	
private:
	
	void initBuffers();
	void loadTexture();
	void loadShaders();
	
	float mix( float x, float y, float a )
	{
		return x * ( 1 - a ) + y * a;
	}
	
	friend class Opponent;
	
private:
	ci::gl::VaoRef		mPVao[2];
	GLuint				mTFOs[2];
	ci::gl::VboRef		mPPositions[2], mPVelocities[2], mPStartTimes[2], mPInitVelocity;
	ci::gl::GlslProgRef	mUpdateOpponentParticlesGlsl, mRenderOpponentParticlesGlsl;
	ci::gl::TextureRef	mSmokeTexture;
	ci::CameraPersp		mCam;
	//float				time;
	//float				deltaT;
	GLuint				renderSub, updateSub, query, drawBuf;
	ci::TriMeshRef		mTrimesh;
    ci::Rand            mRand;
};
	
}