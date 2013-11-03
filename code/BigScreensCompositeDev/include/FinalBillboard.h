//
//  FinalBillboard.h
//  BigScreensComposite
//
//  Created by Ryan Bartley on 11/1/13.
//
//

#pragma once

#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "Utilities.hpp"


namespace bigscreens {
	
typedef std::shared_ptr<class FinalBillboard> FinalBillboardRef;
	
class FinalBillboard {
public:
	FinalBillboard()
	{
		setupBuffers();
		loadShaders();
	}
	
	~FinalBillboard() {}
	
	void setupBuffers()
	{
		float vertices[] = {
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f
		};
		
		uint32_t index[] = {
			0, 1, 2,
			0, 2, 3
		};
		
		mBillboardVao = ci::gl::Vao::create();
		mBillboardVao->bind();
		
		mBillboardVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, 20 * sizeof(float), vertices, GL_STATIC_DRAW );
		mBillboardVbo->bind();
		
		ci::gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
		ci::gl::enableVertexAttribArray(0);
		ci::gl::vertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)) );
		ci::gl::enableVertexAttribArray(1);
		
		mBillboardElementVbo = ci::gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof( uint32_t ), index, GL_STATIC_DRAW );
	}
	
	void loadShaders()
	{
		ci::gl::GlslProg::Format mEffectsFormat;
		mEffectsFormat.vertex( ci::app::loadAsset( SharedShaderAssetPath("finalEffects.vert", !IS_IAC) ) )
		.fragment( ci::app::loadAsset( SharedShaderAssetPath("finalEffects.frag", !IS_IAC) ) );
		mEffectsGlsl = ci::gl::GlslProg::create( mEffectsFormat );
	}
	
	void draw( const ci::gl::TextureRef billboardTex )
	{
		ci::gl::pushMatrices();
		
		mBillboardVao->bind();
		mBillboardElementVbo->bind();
		billboardTex->bind();
		
		mEffectsGlsl->bind();
		mEffectsGlsl->uniform( "fboTexture", 0 );
		mEffectsGlsl->uniform( "texSize", billboardTex->getSize() );
		mEffectsGlsl->uniform( "time", (float)ci::app::getElapsedSeconds() );
		
		ci::gl::drawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
		
		mEffectsGlsl->unbind();
		
		billboardTex->unbind();
		mBillboardElementVbo->unbind();
		mBillboardVao->unbind();
		
		ci::gl::popMatrices();
	}
	
private:
	ci::gl::VaoRef		mBillboardVao;
	ci::gl::VboRef		mBillboardVbo, mBillboardElementVbo;
	ci::gl::GlslProgRef	mEffectsGlsl;
};
	
}
