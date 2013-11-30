//
//  OpponentConent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/21/13.
//
//

#include "OpponentContent.h"
#include "ContentProvider.h"

using namespace bigscreens;
using namespace ci;
using namespace ci::app;

OpponentContent::OpponentContent() :
RenderableContent()
,mOpponent(ActorContentProvider::getOpponent())
,mMinion(ActorContentProvider::getMinion())
{
};

void OpponentContent::load()
{
    loadShaders();
    loadScreen();
}

// TODO: This is duplicated w/ tank content.
// Move this elsewhere.

void OpponentContent::loadShaders()
{
    gl::GlslProg::Format screenShaderFormat;
    screenShaderFormat.vertex( ci::app::loadResource( "offset_texture.vert" ) )
    .fragment( ci::app::loadResource( "offset_texture.frag" ) );
    mTextureShader = ci::gl::GlslProg::create( screenShaderFormat );
    mTextureShader->uniform("uColor", Color::white());
}

void OpponentContent::loadScreen()
{
    mScreenTexture = gl::TextureRef(new gl::Texture(loadImage(app::loadResource("screen.png"))));
    
    GLfloat data[8+8]; // verts, texCoords
    GLfloat *verts = data, *texCoords = data + 8;
    
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
    
    mScreenVao->unbind();
    mScreenVbo->unbind();
}

void OpponentContent::update(std::function<void (ci::CameraPersp & cam, OpponentRef & opponent)> update_func)
{
    mCam.setPerspective( 45.0f, getWindowAspectRatio(), .01, 40000 );
    
    // Sample
    Vec3f smokeDirection(sin(mNumFramesRendered * 0.01),
                         cos(mNumFramesRendered * 0.006666),
                         cos(mNumFramesRendered * 0.003333));
    
    float percentage = sin(mNumFramesRendered * 0.01);
    mOpponent->update(mNumFramesRendered, percentage, smokeDirection);
    
    update_func(mCam, mOpponent);
}

void OpponentContent::render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect)
{
    gl::clear( ColorAf( 0,0,0,0 ) );
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    gl::disableAlphaBlending();
    
    drawScreen(contentRect);
    
    gl::enableDepthWrite();
    gl::enableDepthRead();
    
    gl::pushMatrices();
    
    gl::setMatrices(mCam);
    gl::enableAlphaBlending();
    
    renderOpponent();
    renderMinions();
    
    gl::popMatrices();
}

const static int kNumOpponentMinions = 8;
const static float kMinionIntervalDegrees = 360.0f / kNumOpponentMinions;
const static float kMinionDistance = kOpponentScale * 2.0f;

void OpponentContent::renderOpponent()
{
    gl::pushMatrices();
    gl::scale(Vec3f(kOpponentScale, kOpponentScale, kOpponentScale));
    
    // Arbitrary
    Vec3f lightPos(sin(mNumFramesRendered * 0.1),
                   cos(mNumFramesRendered * 0.06666),
                   cos(mNumFramesRendered * 0.03333));
    
    float zDepth = mCam.getEyePoint().length();
    
    mOpponent->draw(zDepth, lightPos, mNumFramesRendered);
    
    gl::popMatrices();
}

void OpponentContent::renderMinions()
{
    gl::pushMatrices();
    {
        // A simple spin around the opponent.
        gl::bindStockShader(gl::ShaderDef().color());

        for (int i = 0; i < kNumOpponentMinions; ++i)
        {
            gl::pushMatrices();
            
            float radsOffset = toRadians(i * kMinionIntervalDegrees);
            float minionRads = radsOffset + (mNumFramesRendered * -0.01);
            
            Vec3f minionPosition = Vec3f(cos(minionRads) * kMinionDistance,
                                         0,
                                         sin(minionRads) * kMinionDistance);

            gl::translate(minionPosition);
            
            gl::scale(Vec3f(kMinionScale,kMinionScale,kMinionScale));
            gl::color(1.0f, 0.0f, 0.0f, 1.0f);
            gl::setDefaultShaderVars();
            
            /*
            ColorAf roygbivColor(CM_HSV,
                                (float)i / kNumOpponentMinions,
                                1.0f,
                                1.0f,
                                1.0f);
            */
            ColorAf minionColor(0.15,0.15,0.15,1);
            
            mMinion->draw(Vec3f(sin(mNumFramesRendered * 0.1),
                                cos(mNumFramesRendered * 0.06666),
                                cos(mNumFramesRendered * 0.03333)), minionColor);
            
            gl::popMatrices();
        }
    }
    gl::popMatrices();
}

void OpponentContent::drawScreen(const ci::Rectf & contentRect)
{
    gl::pushMatrices();
    
    gl::setMatricesWindow(contentRect.getWidth(), contentRect.getHeight());
    gl::scale(contentRect.getWidth(), contentRect.getHeight());
    
    mTextureShader->bind();
    mScreenTexture->bind();
    
    gl::setDefaultShaderVars();
    
    // No offset
    mTextureShader->uniform("uTexCoordOffset", Vec2f(0,0));
    mTextureShader->uniform("uColor", ColorAf(1,1,1,1));
    
    mScreenVao->bind();
    mScreenVbo->bind();
    
    gl::drawArrays( GL_TRIANGLE_STRIP, 0, 4 );
    
    mScreenVao->unbind();
    mScreenVbo->unbind();
    
    mScreenTexture->unbind();
    mTextureShader->unbind();
    
    gl::popMatrices();
}


ci::Camera & OpponentContent::getCamera()
{
    return mCam;
}
