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
    
    TankContent::TankContent()
    {
    }
    
    void TankContent::load(const std::string & objFilename)
    {
        loadShaders();
        
        loadScreen();
        
        loadObj(objFilename);
        
        loadGround();
        
        // Cam
        mCam.setPerspective( 45.0f, (float)getWindowWidth() / getWindowHeight(), .01, 4000 );
        mCam.lookAt( Vec3f( 0, 200, 1000 ), Vec3f( 0, 100, 0 ) );
        
        mCameraRotation = 0.0f;
    }
    
    void TankContent::loadShaders()
    {
        gl::GlslProg::Format shaderFormat;
        shaderFormat.vertex( ci::app::loadResource( "offset_texture.vert" ) )
        .fragment( ci::app::loadResource( "offset_texture.frag" ) );
        mTextureShader = ci::gl::GlslProg::create( shaderFormat );

        gl::GlslProg::Format mFormat;
        // NOTE: These must be resorces, not assets
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
        
        std::cout << " my mesh is " << mTankMesh->getNumVertices() << " large " << std::endl;
        mTankVao = gl::Vao::create();
        mTankVao->bind();
        
        mTankVbo = gl::Vbo::create(GL_ARRAY_BUFFER, 3 * mTankMesh->getNumVertices() * sizeof(float),
                                   mTankMesh->getVertices<3>(), GL_STATIC_DRAW );
        mTankVbo->bind();
        
        GLint pos = mTankShader->getAttribLocation( "position" );
        gl::enableVertexAttribArray( pos );
        gl::vertexAttribPointer( pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
        
        mTankElementVbo = gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER,
                                          mTankMesh->getNumIndices(),
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
        mGridTexture = gl::TextureRef(new gl::Texture(loadImage(app::loadResource("grid.png")), texFormat));

        GLfloat data[12+8+16]; // verts, texCoords, colors
        GLfloat *verts = data, *texCoords = data + 12, *color = data + 20;
        
        const float kGroundPlaneDimension = 10000.0f;
        const float kGridScale = kGroundPlaneDimension * 0.005f;
        const float kGroundZ = 1.0f;
        const float r = 0.7f, g = 0.7f, b = 0.7f, a = 1.0f;
        for (int i = 0; i < 4; ++i)
        {
            color[i*4+0] = r;
            color[i*4+1] = g;
            color[i*4+2] = b;
            color[i*4+3] = a;
        }
        verts[0*3+0] = kGroundPlaneDimension*0.5f;
        verts[0*3+1] = kGroundPlaneDimension*-0.5f;
        verts[0*3+2] = kGroundZ;
        texCoords[0*2+0] = (mGridTexture->getRight() * kGridScale);
        texCoords[0*2+1] = (mGridTexture->getTop() * kGridScale);

        verts[1*3+0] = kGroundPlaneDimension*-0.5f;
        verts[1*3+1] = kGroundPlaneDimension*-0.5f;
        verts[1*3+2] = kGroundZ;
        texCoords[1*2+0] = (mGridTexture->getLeft() * kGridScale);
        texCoords[1*2+1] = (mGridTexture->getTop() * kGridScale);

        verts[2*3+0] = kGroundPlaneDimension*0.5f;
        verts[2*3+1] = kGroundPlaneDimension*0.5f;
        verts[2*3+2] = kGroundZ;
        texCoords[2*2+0] = (mGridTexture->getRight() * kGridScale);
        texCoords[2*2+1] = (mGridTexture->getBottom() * kGridScale);

        verts[3*3+0] = kGroundPlaneDimension*-0.5f;
        verts[3*3+1] = kGroundPlaneDimension*0.5f;
        verts[3*3+2] = kGroundZ;
        texCoords[3*2+0] = (mGridTexture->getLeft() * kGridScale);
        texCoords[3*2+1] = (mGridTexture->getBottom() * kGridScale);

        mGroundVao = gl::Vao::create();
        mGroundVao->bind();
        mGroundVbo = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW );
        mGroundVbo->bind();
        
        int posLoc = mTextureShader->getAttribSemanticLocation( geom::Attrib::POSITION );
        gl::enableVertexAttribArray( posLoc );
        gl::vertexAttribPointer( posLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
        
        int texLoc = mTextureShader->getAttribSemanticLocation( geom::Attrib::TEX_COORD_0 );
        gl::enableVertexAttribArray( texLoc );
        gl::vertexAttribPointer( texLoc, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*12) );
        
        int colorLoc = mTextureShader->getAttribSemanticLocation( geom::Attrib::COLOR );
        gl::enableVertexAttribArray( colorLoc );
        gl::vertexAttribPointer( colorLoc, 4, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*20) );
        
        mGroundVbo->unbind();
        mGroundVao->unbind();
    }
    
    void TankContent::drawGround()
    {
        gl::pushMatrices();

        gl::setMatrices( mCam );
        gl::rotate(90, 1, 0, 0);
        gl::rotate(90, 0, 0, 1);
        
        // QUESTION:
        // I tried getting the projection and model mat from the cam
        // and passing it into the shader like this:
        // mTextureShader->uniform( "uModelViewProjection", mvpMat);
        // But it had different results than setting the matrix with the
        // camera.
        
        mTextureShader->bind();
        mGridTexture->bind();
        gl::enableAlphaBlending();
        
        // NOTE: This is the speed of the ground.
        static float texOffset = 0.0f;
        texOffset += 0.01f;
        mTextureShader->uniform("uTexCoordOffset", Vec2f(texOffset,0));
        
        mGroundVao->bind();
        mGroundVbo->bind();

        gl::setDefaultShaderVars();
        gl::drawArrays( GL_TRIANGLE_STRIP, 0, 4 );
        
        mGroundVao->unbind();
        mGroundVbo->unbind();

        mGridTexture->unbind();
        mTextureShader->unbind();
        
        gl::popMatrices();
    }
    
    void TankContent::reset()
    {
        mCameraRotation = 0.0f;
    }
    
    void TankContent::update()
    {
         mCameraRotation += 0.01;
         float camX = cosf(mCameraRotation) * 1000;
         float camZ = sinf(mCameraRotation) * 1000;
         mCam.lookAt( Vec3f( camX, 400, camZ ), Vec3f( 0, 100, 0 ) );
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
        
        gl::pushMatrices();
        gl::setMatrices( mCam );
        
        mTankShader->uniform( "projection", gl::getProjection() );
        mTankShader->uniform( "modelView", gl::getModelView() );
        
        gl::drawArrays( GL_LINES, 0, mTankMesh->getNumVertices() );
        //gl::drawElements( GL_LINES, mTankMesh->getNumIndices(), GL_UNSIGNED_INT, 0 );
        gl::popMatrices();
        
        mTankVao->unbind();
        mTankShader->unbind();
        //mTankElementVbo->unbind();
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
