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

namespace bigscreens
{
    
    TankContent::TankContent() :
    mGroundContent(10000.0),
    mTankPosition(0,0,0),
    mIsGroundVisible(true)
    {
    }
    
    void TankContent::setTankPosition(const ci::Vec3f tankPosition)
    {
        mTankPosition = tankPosition;
    }
    
    ci::Vec3f TankContent::getTankPosition()
    {
        return mTankPosition;
    }
    
    void TankContent::setGroundIsVisible(bool isVisible)
    {
        mIsGroundVisible = isVisible;
    }
    
    void TankContent::load(const std::string & objFilename)
    {
        loadShaders();
        
        loadScreen();
        
        loadObj(objFilename);
        
        loadGround();
        
        // Cam
        mCam.lookAt( Vec3f( 0, 200, 1000 ), Vec3f( 0, 100, 0 ) );
        
        mGroundOffset = Vec2f::zero();
    }
    
    void TankContent::loadShaders()
    {
        gl::GlslProg::Format screenShaderFormat;
        screenShaderFormat.vertex( ci::app::loadResource( "offset_texture.vert" ) )
        .fragment( ci::app::loadResource( "offset_texture.frag" ) );
        mTextureShader = ci::gl::GlslProg::create( screenShaderFormat );
        
        gl::GlslProg::Format groundShaderFormat;
        groundShaderFormat.vertex( ci::app::loadResource( "ground_texture.vert" ) )
        .fragment( ci::app::loadResource( "ground_texture.frag" ) );
        mGroundShader = ci::gl::GlslProg::create( groundShaderFormat );

        gl::GlslProg::Format mFormat;
        mFormat.vertex( loadResource( "basic.vert" ) )
        .fragment( loadResource( "basic.frag" ) );
        mTankShader = gl::GlslProg::create( mFormat );
        mTankShader->bind();
    }
    
    void TankContent::loadScreen()
    {
        mScreenTexture = gl::TextureRef(new gl::Texture(loadImage(app::loadResource("screen.png"))));
        
        GLfloat data[8+8+16]; // verts, texCoords, colors
        GLfloat *verts = data, *texCoords = data + 8, *color = data + 16;
        const float r = 1.f, g = 1.f, b = 1.f, a = 1.f;
        for (int i = 0; i < 4; ++i)
        {
            color[i*4+0] = r;
            color[i*4+1] = g;
            color[i*4+2] = b;
            color[i*4+3] = a;
        }
        verts[0*2+0] = 1.0f;
        verts[0*2+1] = 0.0f;
        texCoords[0*2+0] = mScreenTexture->getRight();
        texCoords[0*2+1] = mScreenTexture->getTop();
        
        verts[1*2+0] = 0.0f;
        verts[1*2+1] = 0.0f;
        texCoords[1*2+0] = mScreenTexture->getLeft();
        texCoords[1*2+1] = mScreenTexture->getTop();
        
        verts[2*2+0] = 1.0f;
        verts[2*2+1] = 1.0f;
        texCoords[2*2+0] = mScreenTexture->getRight();
        texCoords[2*2+1] = mScreenTexture->getBottom();
        
        verts[3*2+0] = 0.0f;
        verts[3*2+1] = 1.0f;
        texCoords[3*2+0] = mScreenTexture->getLeft();
        texCoords[3*2+1] = mScreenTexture->getBottom();
        
        mScreenVao = gl::Vao::create();
        mScreenVao->bind();
        mScreenVbo = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW );
        mScreenVbo->bind();
        
        int posLoc = mTextureShader->getAttribSemanticLocation( geom::Attrib::POSITION );
        gl::enableVertexAttribArray( posLoc );
        gl::vertexAttribPointer( posLoc, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
        
        int texLoc = mTextureShader->getAttribSemanticLocation( geom::Attrib::TEX_COORD_0 );
        gl::enableVertexAttribArray( texLoc );
        gl::vertexAttribPointer( texLoc, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*8) );
        
        int colorLoc = mTextureShader->getAttribSemanticLocation( geom::Attrib::COLOR );
        gl::enableVertexAttribArray( colorLoc );
        gl::vertexAttribPointer( colorLoc, 4, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*16) );
        
        mScreenVao->unbind();
        mScreenVbo->unbind();
    }
    
    void TankContent::loadObj(const std::string & objFilename)
    {
        DataSourceRef file = loadResource( objFilename ); //"T72.obj"
        ObjLoader loader( file );
        mTankMesh = TriMesh::create( loader );
        
        mTankVao = gl::Vao::create();
        mTankVao->bind();
        
        mTankVbo = gl::Vbo::create(GL_ARRAY_BUFFER, 3 * mTankMesh->getNumVertices() * sizeof(float),
                                   mTankMesh->getVertices<3>(), GL_STATIC_DRAW );
        mTankVbo->bind();
        
        GLint pos = mTankShader->getAttribLocation( "vPosition" );
        gl::enableVertexAttribArray( pos );
        gl::vertexAttribPointer( pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
        
        mTankElementVbo = gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER,
                                          mTankMesh->getNumIndices() * 4,
                                          mTankMesh->getIndices().data());
        mTankElementVbo->bind();
        mTankElementVbo->unbind();
        
        mTankVbo->unbind();
        mTankVao->unbind();
        mTankShader->unbind();
    }
    
    void TankContent::loadGround()
    {
        gl::Texture::Format texFormat;
        texFormat.setWrap(GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T);
        texFormat.mipMap(true);
        mGridTexture = gl::TextureRef(new gl::Texture(loadImage(app::loadResource("grid.png")), texFormat));
        
        mGroundContent.load(mGroundShader);
    }
    
    void TankContent::drawGround()
    {
        if (!mIsGroundVisible) return;
        
        gl::pushMatrices();

        gl::setMatrices( mCam );

        mGroundShader->bind();
        mGridTexture->bind();
        
        // Probably not necessary
        gl::enableAlphaBlending();
        
        mGroundShader->uniform("uTexCoordOffset", mGroundOffset);
        mGroundShader->uniform("uColor", ColorAf(0.75,0.75,0.75,1.0f));
        mGroundShader->uniform("uTexScale", 50.f);
        
        // Get the current plot
        float groundScale = mGroundContent.getScale();
        Vec3f groundOffset((-0.5f * groundScale),
                           0,
                           (-0.5f * groundScale));

        mGroundContent.render(GL_TRIANGLE_STRIP, groundOffset);
        
        mGridTexture->unbind();
        mTextureShader->unbind();
        
        gl::popMatrices();
    }
    
    void TankContent::reset()
    {
        mGroundOffset = Vec2f::zero();
        resetPositions();
    }
    
    void TankContent::resetPositions()
    {
        mCam.setPerspective( 45.0f, (float)getWindowWidth() / getWindowHeight(), .01, 40000 );
        mTankPosition = Vec3f::zero();
    }
    
    void TankContent::setGroundOffset(const Vec2f offset)
    {
        mGroundOffset = offset;
    }

    // Lets the app take control of the cam
    void TankContent::update(std::function<void (ci::CameraPersp & cam)> update_func)
    {
        update_func(mCam);
    }
    
    void TankContent::drawScreen()
    {
        gl::pushMatrices();
        
        gl::setMatricesWindow(getWindowWidth(), getWindowHeight());
        gl::scale(getWindowWidth(), getWindowHeight());
        
        mTextureShader->bind();
        mScreenTexture->bind();

        // No offset
        mTextureShader->uniform("uTexCoordOffset", Vec2f(0,0));
        
        mScreenVao->bind();
        mScreenVbo->bind();
        
        gl::setDefaultShaderVars();
        gl::drawArrays( GL_TRIANGLE_STRIP, 0, 4 );
        
        mScreenVao->unbind();
        mScreenVbo->unbind();
        
        mScreenTexture->unbind();
        mTextureShader->unbind();
        
        gl::popMatrices();
    }
    
    void TankContent::drawTank()
    {
        mTankShader->bind();
        mTankVao->bind();
        mTankElementVbo->bind();
        
        gl::pushMatrices();
        
        gl::setMatrices( mCam );
        gl::translate(mTankPosition);
        
        gl::setDefaultShaderVars();
        
        mTankShader->uniform("uColor", ColorAf(1,1,1,1));
        
        gl::drawElements( GL_LINES, mTankMesh->getNumIndices(), GL_UNSIGNED_INT, 0 );
        gl::popMatrices();
        
        mTankElementVbo->unbind();
        mTankVao->unbind();
        mTankShader->unbind();
    }
    
    void TankContent::render(const ci::Vec2i & screenOffset)
    {
        // clear out both of the attachments of the FBO with black
        gl::clear( ColorAf( 0.0f, 0.0f, 0.0f, 0.0f ) );
        
        drawScreen();
        
        drawGround();

        drawTank();
    }
}
