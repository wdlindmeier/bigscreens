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
#include "cinder/Surface.h"
#include "cinder/Utilities.h"
#include "cinder/Camera.h"
#include "Utilities.hpp"
#include "ContentProvider.h"

using namespace ci;
using namespace ci::app;
using namespace bigscreens;

namespace bigscreens
{
    
    TankContent::TankContent() :
    RenderableContent()
    , mTankDirectionRadians(0)
    , mTankPosition(0,0,0)
    , mIsGroundVisible(true)
    , mTank( ContentProviderNew::ActorContent::getAdvancedTank() )
    , mMinion( ContentProviderNew::ActorContent::getMinion() )
    , mGroundPlane( ContentProviderNew::ActorContent::getFloorPlane() )
    , mGroundPlotCoords(0,0,0)
    , mGroundScale(10000, 500, 10000) // Keep these symetrical x/z
    {
    }
    
    void TankContent::setFrameContentID(const int contentID)
    {
        RenderableContent::setFrameContentID(contentID);
        mTank->setFrameContentID(contentID);
    };
    
    void TankContent::setTankPosition(const ci::Vec3f tankPosition)
    {
        mTankPosition = tankPosition;
    }
    
    ci::Vec3f TankContent::getTankPosition()
    {
        return mTankPosition;
    }
    
    ci::CameraPersp & TankContent::getCamera()
    {
        return mCam;
    }
    
    AdvancedTankRef & TankContent::getTank()
    {
        return mTank;
    }
    
    void TankContent::setGroundIsVisible(bool isVisible)
    {
        mIsGroundVisible = isVisible;
    }
    
    void TankContent::load(const std::string & objFilename)
    {
        
        mPerlinContent.reset();

        loadShaders();
        
        loadScreen();

        mCam.lookAt( Vec3f( 0, 200, 1000 ), Vec3f( 0, 100, 0 ) );

    }
    
    void TankContent::loadShaders()
    {
        gl::GlslProg::Format screenShaderFormat;
        screenShaderFormat.vertex( ci::app::loadResource( "offset_texture.vert" ) )
        .fragment( ci::app::loadResource( "offset_texture.frag" ) );
        mTextureShader = ci::gl::GlslProg::create( screenShaderFormat );
        mTextureShader->uniform("uColor", Color::white());
    }
    
    void TankContent::loadScreen()
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

    // This is kinda crazy. A quick way of getting a map key from a plot position.
    static float KeyFromPlot(int x, int z)
    {
        return (float)x + (z * 0.001);
    }
    
    void TankContent::generateGroundMaps()
    {
        // mGroundMaps.clear();
        
        // NOTE: This lets us cache all of the textures,
        // but depending upon how long each scene runs, this might
        // gobble up a bunch of memory... probably not a huge deal.
        
        Vec2i texSize = mPerlinContent.getTexture()->getSize();
        
        for (int i = 0; i < 9; ++i)
        {
            int x = (i % 3) - 1;
            int z = ((i / 3) - 1) * -1;
            int plotX = mGroundPlotCoords.x + x;
            int plotZ = mGroundPlotCoords.z + z;
            
            //Vec2i plot(plotX, plotZ);
            float key = KeyFromPlot(plotX, plotZ);
            
            if (mGroundMaps.find(key) == mGroundMaps.end() )
            {
                // Didn't find the texture.
                // Generate now
                ci::app::console() << "Generating new ground\n";
                
                mPerlinContent.generateNoiseForPosition(Vec2f(texSize.x * plotX,
                                                              texSize.y * plotZ));
                gl::TextureRef newTex = mPerlinContent.getTextureRef();
                mGroundMaps[key] = newTex;
            }
        }
    }
    
    void TankContent::drawGround()
    {
        if (!mIsGroundVisible) return;

        // Get the current plot
        int plotX = (mTankPosition.x + (mGroundScale.x * 0.5f)) / mGroundScale.x;
        int plotZ = (mTankPosition.z + (mGroundScale.z * 0.5f)) / mGroundScale.z;
        
        if (mGroundPlotCoords.x != plotX || mGroundPlotCoords.z != plotZ || mGroundMaps.size() == 0)
        {
            mGroundPlotCoords = Vec3i(plotX, 0, plotZ);
            generateGroundMaps();
        }
        
        drawGroundTile(mGroundPlotCoords + Vec3i(-1, 0, 1), mGroundMaps[KeyFromPlot(plotX-1, plotZ+1)]);
        drawGroundTile(mGroundPlotCoords + Vec3i(0, 0, 1), mGroundMaps[KeyFromPlot(plotX, plotZ+1)]);
        drawGroundTile(mGroundPlotCoords + Vec3i(1, 0, 1), mGroundMaps[KeyFromPlot(plotX+1, plotZ+1)]);
        
        drawGroundTile(mGroundPlotCoords + Vec3i(-1, 0, 0), mGroundMaps[KeyFromPlot(plotX-1, plotZ)]);
        drawGroundTile(mGroundPlotCoords + Vec3i(0, 0, 0), mGroundMaps[KeyFromPlot(plotX, plotZ)]);
        drawGroundTile(mGroundPlotCoords + Vec3i(1, 0, 0), mGroundMaps[KeyFromPlot(plotX+1, plotZ)]);
        
        drawGroundTile(mGroundPlotCoords + Vec3i(-1, 0, -1), mGroundMaps[KeyFromPlot(plotX-1, plotZ-1)]);
        drawGroundTile(mGroundPlotCoords + Vec3i(0, 0, -1), mGroundMaps[KeyFromPlot(plotX, plotZ-1)]);
        drawGroundTile(mGroundPlotCoords + Vec3i(1, 0, -1), mGroundMaps[KeyFromPlot(plotX+1, plotZ-1)]);
    }
    
    void TankContent::drawGroundTile(const ci::Vec3i & plot, gl::TextureRef & heightMap)
    {
        gl::disableAlphaBlending();
        
        gl::pushMatrices();
        gl::setMatrices( mCam );
        // Scale to taste
        gl::scale(mGroundScale);
        // Center
        gl::translate(Vec3f(-0.5 + plot.x,
                            0,
                            -0.5 + plot.z));
        
        mGroundPlane->setNoiseTexture(heightMap);

		mGroundPlane->draw(mNumFramesRendered, false, ColorAf(0.5,0.5,0.5,1)); //true);
        
        gl::popMatrices();
        
    }
    
    void TankContent::reset()
    {
        resetPositions();
    }
    
    void TankContent::resetPositions()
    {
        mCam.setPerspective( 45.0f, getWindowAspectRatio(), .01, 40000 );
        mTank->setWheelSpeedMultiplier(kDefaultTankWheelSpeedMulti);
        mTankPosition = Vec3f::zero();
    }

    // Lets the app take control of the cam.
    void TankContent::update(std::function<void (ci::CameraPersp & cam, AdvancedTankRef & tank)> update_func)
    {
        // TODO: Make the minion behavior more interesting
        Vec3f minionTargetPosition = mTankPosition + Vec3f(cos(mNumFramesRendered * -0.01) * 2000,
                                                           800,
                                                           sin(mNumFramesRendered * -0.01) * 2000);        
        mMinionPosition = minionTargetPosition;
        
        // NOTE: The target position is relative to the tank
        mTank->setTargetPosition(mMinionPosition - mTankPosition);
        
        update_func(mCam, mTank);
        
        // NOTE: Wheel speed comes from the update llamba, so keep
        // tank update after that.
        mTank->update(mNumFramesRendered);

    }
    
    void TankContent::drawScreen(const ci::Rectf & contentRect)
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
    
    void TankContent::drawTank()
    {
        gl::pushMatrices();
        gl::setMatrices( mCam );
        
        Vec3f prevGroundRelationship = Vec3f::zero();
        if (mTankGroundRelationships.find(mContentID) != mTankGroundRelationships.end())
        {
            prevGroundRelationship = mTankGroundRelationships[mContentID];
        }
        Vec3f curGroundRelationship = prevGroundRelationship;

        gl::pushMatrices();
        gl::translate(mTankPosition);
        
        float key = KeyFromPlot(mGroundPlotCoords.x,
                                mGroundPlotCoords.z);
        float height = 0;
        if (mGroundMaps.find(key) != mGroundMaps.end())
        {
            // NOTE: This is just used to get the dimensions of the texture
            // gl::TextureRef currentGround = mGroundMaps[key];
            Vec2i textureSize = mGroundMaps[key]->getSize();
            
            // Get Model Sampling Coords
            float xPosLeft = kTankBodyWidth*-0.5;
            float xPosRight = kTankBodyWidth*0.5;
            float zPosRear = kTankBodyHeadToToe * kTankCenterZOffset * -1;
            float zPosFore = kTankBodyHeadToToe * (1.0-kTankCenterZOffset);
            
            float xMid = (xPosLeft + xPosRight) / 2.0f;
            float zMid = (zPosRear + zPosFore) / 2.0f;
            
            Vec2f tankPosRear(xMid, zPosRear);
            Vec2f tankPosFore(xMid, zPosFore);
            Vec2f tankPosLeft(xPosLeft, zMid);
            Vec2f tankPosRight(xPosRight, zMid);
            
            // Rotate arond Zero depending upon the direction
            tankPosRear.rotate(-mTankDirectionRadians);
            tankPosFore.rotate(-mTankDirectionRadians);
            tankPosLeft.rotate(-mTankDirectionRadians);
            tankPosRight.rotate(-mTankDirectionRadians);
            
            // Convert to World coords by adding tank position
            Vec2f totalOffset = mTankPosition.xz() + (mGroundScale.xz() * 0.5f);
            tankPosRear += totalOffset;
            tankPosFore += totalOffset;
            tankPosLeft += totalOffset;
            tankPosRight += totalOffset;
            
            // Tex / Noise Coords
            Vec2f groundToTexScale = Vec2f(textureSize) / mGroundScale.xz();
            float depthSampleRear = mPerlinContent.getValueAtPosition(tankPosRear * groundToTexScale);
            float depthSampleFore = mPerlinContent.getValueAtPosition(tankPosFore * groundToTexScale);
            float depthSampleLeft = mPerlinContent.getValueAtPosition(tankPosLeft * groundToTexScale);
            float depthSampleRight = mPerlinContent.getValueAtPosition(tankPosRight * groundToTexScale);
            
            float heightRear = depthSampleRear * mGroundScale.y;
            float heightFore = depthSampleFore * mGroundScale.y;
            float heightLeft = depthSampleLeft * mGroundScale.y;
            float heightRight = depthSampleRight * mGroundScale.y;

            height = (heightRear + heightFore + heightLeft + heightRight) / 4.f;
            
            // Get the angle
            float zOffset = kTankBodyHeadToToe;
            float yOffset = heightFore - heightRear;
            float radsAngleX = atan2f(zOffset, yOffset);
            
            float xOffset = kTankBodyWidth;
            yOffset = heightLeft - heightRight;
            float radsAngleZ = atan2f(xOffset, yOffset);
            
            // Average
            float prevOrientationWeight = 2.0f;
            float newHeight = (prevGroundRelationship.y * prevOrientationWeight + height) / (prevOrientationWeight + 1.0f);
            float newAngleX = (prevGroundRelationship.x * prevOrientationWeight + radsAngleX) / (prevOrientationWeight + 1.0f);
            float newAngleZ = (prevGroundRelationship.z * prevOrientationWeight + radsAngleZ) / (prevOrientationWeight + 1.0f);
            
            // Perhaps we should use a more semantically meaningful struct
            curGroundRelationship.x = newAngleX;
            curGroundRelationship.y = newHeight;
            curGroundRelationship.z = newAngleZ;

            // Adjust the height
            gl::translate(Vec3f(0,curGroundRelationship.y,0));
            
            // Adjust the X angle
            gl::rotate(toDegrees(curGroundRelationship.x) - 90, 1, 0, 0);

            // Adjsut the Z angle
            gl::rotate(toDegrees(curGroundRelationship.z) - 90, 0, 0, 1);
            
        }
        else
        {
            // console() << "WARN: Cant find ground texture\n";
        }
        
        mTank->render();
        gl::popMatrices();

        mTank->renderShots(mCam);
        gl::popMatrices();
        
        mTankGroundRelationships[mContentID] = curGroundRelationship;
    }
    
    void TankContent::drawMinion()
    {
        // A simple spin around the tank.
        // TODO: Make this more interesting
        
        gl::bindStockShader(gl::ShaderDef().color());

        gl::pushMatrices();
        gl::setMatrices( mCam );
        
        // Spin baby
        // TODO: Give this realistic movement
        gl::translate(mMinionPosition);
        
        gl::scale(Vec3f(150,150,150));
        gl::color(1, 0, 0);
        gl::setDefaultShaderVars();

        mMinion->draw(Vec3f::zero(), ColorAf(1,0,0,1));

        gl::popMatrices();
    }
    
    void TankContent::render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect)
    {
        if (mContentID == -1)
        {
            console() << "WARN: Tank content doesn't have a contentID\n";
        }
        
        // clear out both of the attachments of the FBO with black
        gl::clear( ColorAf( 0.0f, 0.0f, 0.0f, 0.0f ) );
        
        drawScreen(contentRect);
        
        gl::enableDepthRead();
        gl::enableDepthWrite();
        
        drawGround();
        
        drawTank();
        
        drawMinion();
        
        // Clear out the content ID.
        // Do it through the getter.
        setFrameContentID(-1);
    }
}
