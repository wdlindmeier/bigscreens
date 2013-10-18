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
        mScreenTexture = gl::TextureRef(new gl::Texture(loadImage(app::loadResource("screen.png"))));
        
        gl::GlslProg::Format mFormat;
        // NOTE: These must be resorces, not assets
        mFormat.vertex( loadResource( "basic.vert" ) )
        .fragment( loadResource( "basic.frag" ) );
        mGlsl = gl::GlslProg::create( mFormat );
        mGlsl->bind();
        
        DataSourceRef file = loadResource( objFilename ); //"T72.obj"
        ObjLoader loader( file );
        mMesh = TriMesh::create( loader );
        
        std::cout << " my mesh is " << mMesh->getNumVertices() << " large " << std::endl;
        mVao = gl::Vao::create();
        mVao->bind();
        
        mVbo = gl::Vbo::create( GL_ARRAY_BUFFER, 3 * mMesh->getNumVertices() * sizeof(float),
                               mMesh->getVertices<3>(), GL_STATIC_DRAW );
        mVbo->bind();
        
        GLint pos = mGlsl->getAttribLocation( "position" );
        gl::enableVertexAttribArray( pos );
        gl::vertexAttribPointer( pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
        
        //	mElementVbo = gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, mMesh->getNumIndices(), mMesh->getIndices().data() );
        //	mElementVbo->bind();
        //	mElementVbo->unbind();
        
        mVbo->unbind();
        mVao->unbind();
        mGlsl->unbind();
        
        mCam.setPerspective( 45.0f, (float)getWindowWidth() / getWindowHeight(), .01, 4000 );
        mCam.lookAt( Vec3f( 0, 200, 1000 ), Vec3f( 0, 100, 0 ) );
        
        mCameraRotation = 0.0f;
    }
    
    void TankContent::reset()
    {
        //mRotation.setToIdentity();
        mCameraRotation = 0.0f;
    }
    
    void TankContent::update()
    {
        //mRotation.rotate( Vec3f( 0, 1, 0 ), 0.006f );
        //mCam.lookAt(<#const Vec3f &aEyePoint#>, <#const Vec3f &target#>)
        
        // Rather than changing the matrix, change the camera position
        // eye, target
        mCameraRotation += 0.01;
        float camX = cosf(mCameraRotation) * 1000;
        float camZ = sinf(mCameraRotation) * 1000;
        mCam.lookAt( Vec3f( camX, 400, camZ ), Vec3f( 0, 100, 0 ) );
    }
    
    void TankContent::render(const ci::Vec2i & screenOffset)
    {
        // clear out both of the attachments of the FBO with black
        gl::clear( ColorAf( 0.0f, 0.0f, 0.0f, 0.0f ) );
        
        gl::enableAdditiveBlending();
        Rectf screenRect(0,0,getWindowWidth(), getWindowHeight());
        gl::draw(mScreenTexture, screenRect);
        
        mGlsl->bind();
        mVao->bind();
        //	mElementVbo->bind();
        
        gl::pushMatrices();
        gl::setMatrices( mCam );
        //gl::multModelView( mRotation );
        mGlsl->uniform( "projection", gl::getProjection() );
        mGlsl->uniform( "modelView", gl::getModelView() );
        
        gl::drawArrays( GL_LINES, 0, mMesh->getNumVertices() );
        //	gl::drawElements( GL_TRIANGLES, mMesh->getNumIndices(), GL_UNSIGNED_INT, 0 );
        gl::popMatrices();
        
        mVao->unbind();
        mGlsl->unbind();
        //	mElementVbo->unbind();
        
    }
	
}
