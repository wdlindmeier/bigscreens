//
//  FinalBillboard.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/27/13.
//
//

#include "FinalBillboard.h"
#include "ContentProvider.h"

using namespace bigscreens;

FinalBillboard::FinalBillboard()
{
    setupBuffers();
    loadShaders();
}

void FinalBillboard::setupBuffers()
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
	
void FinalBillboard::loadShaders()
{
    ci::gl::GlslProg::Format mEffectsFormat;
    mEffectsFormat.vertex( ci::app::loadResource( "finalEffects.vert" ) )
    .fragment( ci::app::loadResource( "finalEffects.frag" ) );
    
    mEffectsGlsl = ci::gl::GlslProg::create( mEffectsFormat );
}
	
void FinalBillboard::draw( const ci::gl::TextureRef billboardTex, long numFramesRendered )
{    
    ci::gl::pushMatrices();
    ci::gl::viewport( ci::Vec2i(), billboardTex->getSize() );
    
    mBillboardVao->bind();
    mBillboardElementVbo->bind();
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, billboardTex->getId() );
    
    mEffectsGlsl->bind();
    mEffectsGlsl->uniform( "fboTexture", 0 );
    //		mEffectsGlsl->uniform( "texSize", billboardTex->getSize() );
    // TODO
    TryAddingUniform(mEffectsGlsl, "time", (float)(numFramesRendered / 60.0f));
    //mEffectsGlsl->uniform( "time", (float)ci::app::getElapsedSeconds() );
    
    // NOTE: Channel 20 seems like a good sampling band
    const int kObserveFFTChannel = 20;
    float fftVolume = SceneContentProvider::sharedContentProvider()->getFFTDataForChannel(kObserveFFTChannel);
    // Weight it toward 0
    fftVolume = fftVolume*fftVolume*fftVolume*fftVolume*fftVolume;

    TryAddingUniform(mEffectsGlsl, "volume", 6.0f * fftVolume);
    
    //mEffectsGlsl->uniform( "volume", 0.0f);
    
    ci::gl::drawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
    
    mEffectsGlsl->unbind();
    
    glBindTexture( GL_TEXTURE_2D, 0 );
    mBillboardElementVbo->unbind();
    mBillboardVao->unbind();
    
    ci::gl::popMatrices();
}
