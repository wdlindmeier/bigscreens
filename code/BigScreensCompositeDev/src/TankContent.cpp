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
#include "FiringTank.h"
#include "ContentProvider.h"

using namespace ci;
using namespace ci::app;
using namespace bigscreens;

const static float kDefaultGroundScaleX = 10000;
const static float kDefaultGroundScaleY = 500;
const static float kDefaultGroundScaleZ = kDefaultGroundScaleX;

namespace bigscreens
{
    
    TankContent::TankContent() :
    RenderableContent()
    , mIsGroundVisible(true)
    , mTank( ActorContentProvider::getAdvancedTank() )
    , mMinion( ActorContentProvider::getMinion() )
    , mGroundPlane( ActorContentProvider::getFloorPlane() )
    , mGroundPlotCoords(0,0,0)
    , mMountainMagnitude(5.0f)
    , mGroundScale(kDefaultGroundScaleX, kDefaultGroundScaleY, kDefaultGroundScaleZ)
    {
    }
    
    void TankContent::setFrameContentID(const int contentID)
    {
        RenderableContent::setFrameContentID(contentID);
        mTank->setFrameContentID(contentID);
    };
    
    void TankContent::setGroundScale(const ci::Vec3f & groundScale)
    {
        mGroundScale = groundScale;
    }
    
    void TankContent::setMountainMagnitude(const float magnitude)
    {
        mMountainMagnitude = magnitude;
    }
    
    void TankContent::setTankPosition(const ci::Vec3f tankPosition, const float directionRadians)
    {
        if (mContentID == -1)
        {
            console() << "ERROR: Can't set position. Tank content doesn't have a contentID\n";
        }
        else
        {
            PositionOrientation orientation;
            orientation.position = tankPosition;
            orientation.directionDegrees = toDegrees(directionRadians);
            
            Vec3f vector(0,0,0);
            if (mPositionOrientations.find(mContentID) != mPositionOrientations.end())
            {
                PositionOrientation prevOrientation = mPositionOrientations[mContentID];
                vector = tankPosition - prevOrientation.position;
            }
            orientation.vector = vector;
            
            mPositionOrientations[mContentID] = orientation;
            // update ground orientation
            updateGroundOrientationWithCurrentPosition();
        }
    }
    
    ci::Vec3f TankContent::getTankPosition()
    {
        if (mContentID == -1)
        {
            console() << "ERROR: Can't find tank position. Tank content doesn't have a contentID\n";
            return Vec3f::zero();
        }
        PositionOrientation orientation = mPositionOrientations[mContentID];
        return orientation.position;
    }
    
    ci::Vec3f TankContent::getTankVector()
    {
        if (mContentID == -1)
        {
            console() << "ERROR: Can't find tank vector. Tank content doesn't have a contentID\n";
            return Vec3f::zero();
        }
        PositionOrientation orientation = mPositionOrientations[mContentID];
        return orientation.vector;
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
    
    void TankContent::load()
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

    void TankContent::generateGroundMapForPlot(const ci::Vec3i & plot)
    {
        float key = KeyFromPlot(plot.x, plot.z);
        Vec2i texSize = mPerlinContent.getTextureSize();
        if (mGroundMaps.find(key) == mGroundMaps.end() )
        {
            // Didn't find the texture.
            // Generate now
            ci::app::console() << "Generating new ground for key " << key << "\n";
            mPerlinContent.generateNoiseForPosition(Vec2f(texSize.x * plot.x,
                                                          texSize.y * plot.z));
            gl::TextureRef newTex = mPerlinContent.getTextureRef();
            mGroundMaps[key] = newTex;
        }
    }
    
    void TankContent::updateGroundCoordsForTank()
    {
        Vec3f tankPosition = getTankPosition();
        
        // Get the current plot
        int plotX = (tankPosition.x + (mGroundScale.x * 0.5f)) / mGroundScale.x;
        int plotZ = (tankPosition.z + (mGroundScale.z * 0.5f)) / mGroundScale.z;
        
        if (mGroundPlotCoords.x != plotX || mGroundPlotCoords.z != plotZ || mGroundMaps.size() == 0)
        {
            mGroundPlotCoords = Vec3i(plotX, 0, plotZ);
        }
        
        // Pass the tank pos and vector into the ground

    }
    
    void TankContent::drawGround()
    {
        if (!mIsGroundVisible) return;
        
        updateGroundCoordsForTank();

        drawGroundTile(mGroundPlotCoords + Vec3i(-1, 0, 1));
        drawGroundTile(mGroundPlotCoords + Vec3i(0, 0, 1));
        drawGroundTile(mGroundPlotCoords + Vec3i(1, 0, 1));
        
        drawGroundTile(mGroundPlotCoords + Vec3i(-1, 0, 0));
        drawGroundTile(mGroundPlotCoords + Vec3i(0, 0, 0));
        drawGroundTile(mGroundPlotCoords + Vec3i(1, 0, 0));
        
        drawGroundTile(mGroundPlotCoords + Vec3i(-1, 0, -1));
        drawGroundTile(mGroundPlotCoords + Vec3i(0, 0, -1));
        drawGroundTile(mGroundPlotCoords + Vec3i(1, 0, -1));

    }
    
    void TankContent::drawGroundTile(const ci::Vec3i & plot)
    {
        float plotKey = KeyFromPlot(plot.x, plot.z);

        if (mGroundMaps.find(plotKey) == mGroundMaps.end())
        {
            generateGroundMapForPlot(plot);
        }
        gl::TextureRef & heightMap = mGroundMaps[plotKey];
        assert(heightMap);
        
        gl::pushMatrices();
        gl::setMatrices( mCam );
        // Scale to taste
        gl::scale(mGroundScale);
        // Center
        Vec3f groundOffset(-0.5 + plot.x, 0, -0.5 + plot.z);
        gl::translate(groundOffset);
        
        mGroundPlane->setNoiseTexture(heightMap);

        Vec3f tankPosition = getTankPosition();
        Vec3f tankVector = getTankVector();

        float mountainMagnitude = mMountainMagnitude;
        if (tankVector == Vec3f::zero())
        {
            mountainMagnitude = 0.0f;
        }
		mGroundPlane->draw(mNumFramesRendered,
                           mRenderAlpha,
                           mountainMagnitude,
                           mGroundScale,
                           groundOffset,
                           tankPosition,
                           tankVector.normalized());
        
        gl::popMatrices();
    }
    
    void TankContent::fireTankGun()
    {
        fireTankGun(mTank);
    }
    
    void TankContent::fireTankGun(FiringTankRef firingTank)
    {
        if (mContentID == -1)
        {
            console() << "ERROR: Tank content doesn't have a contentID\n";
            return;
        }
            
        GroundOrientaion curGroundOrientation;
        PositionOrientation position;
        if (mTankGroundOrientations.find(mContentID) != mTankGroundOrientations.end())
        {
            curGroundOrientation = mTankGroundOrientations[mContentID];
        }
        else
        {
            console() << "ERROR: Couldn't find ground orientation for firing\n";
        }
        
        if (mPositionOrientations.find(mContentID) != mPositionOrientations.end())
        {
            position = mPositionOrientations[mContentID];
        }
        else
        {
            console() << "ERROR: Couldn't find position for firing\n";
        }
        // console() << "Firing from position: " << position.position << "\n";

        firingTank->fire(position, curGroundOrientation);
    }
    
    void TankContent::reset()
    {
        resetPositions();
    }

    void TankContent::setDefaultGroundScale()
    {
        mGroundScale = Vec3f(kDefaultGroundScaleX, kDefaultGroundScaleY, kDefaultGroundScaleZ);
    }
    
    void TankContent::resetPositions()
    {
        mCam.setPerspective( 45.0f, getWindowAspectRatio(), .01, 40000 );
        mTank->setWheelSpeedMultiplier(kDefaultTankWheelSpeedMulti);
        //mTankPosition = Vec3f::zero();
    }

    // Lets the app take control of the cam.
    void TankContent::update(std::function<void (ci::CameraPersp & cam, AdvancedTankRef & tank)> update_func)
    {
        Vec3f tankPosition = getTankPosition();
        // TODO: Make the minion behavior more interesting
        Vec3f minionTargetPosition = tankPosition + Vec3f(cos(mNumFramesRendered * -0.01) * 2000,
                                                           800,
                                                           sin(mNumFramesRendered * -0.01) * 2000);        
        mMinionPosition = minionTargetPosition;
        
        // NOTE: The target position is relative to the tank
        mTank->setTargetPosition(mMinionPosition - tankPosition);

        // NOTE: Keep this before the update llambda so the user has access to the current state
        mTank->update(mNumFramesRendered);
        
        // updateGroundOrientationWithCurrentPosition();
        
        update_func(mCam, mTank);
        // Nothing should happen after update_func
    }
    
    void TankContent::updateGroundOrientationWithCurrentPosition()
    {
        // NOTE: This assumes the tank position has already been set
        PositionOrientation tankOrient = mPositionOrientations[mContentID];
        GroundOrientaion groundOrient = groundOrientationForPosition(tankOrient);
        mTankGroundOrientations[mContentID] = groundOrient;
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
        mTextureShader->uniform("uColor", ColorAf(1,1,1,mScreenAlpha));

        mScreenVao->bind();
        mScreenVbo->bind();
        
        gl::drawArrays( GL_TRIANGLE_STRIP, 0, 4 );
        
        mScreenVao->unbind();
        mScreenVbo->unbind();
        
        mScreenTexture->unbind();
        mTextureShader->unbind();
        
        gl::popMatrices();
    }
    
    GroundOrientaion TankContent::groundOrientationForPosition(const PositionOrientation & position)
    {
        GroundOrientaion groundOrient;
        
        float height = 0;
        
        Vec2i textureSize = mPerlinContent.getTextureSize();
        
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
        float directionRads = toRadians(position.directionDegrees);
        tankPosRear.rotate(-directionRads);
        tankPosFore.rotate(-directionRads);
        tankPosLeft.rotate(-directionRads);
        tankPosRight.rotate(-directionRads);
        
        // Convert to World coords by adding tank position
        Vec2f totalOffset = position.position.xz() + (mGroundScale.xz() * 0.5f);
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
        
        // Update the orientation
        groundOrient.xAngleRads = radsAngleX;
        groundOrient.height = height;
        groundOrient.zAngleRads = radsAngleZ;
        
        return groundOrient;
    }
    
    void TankContent::drawTank()
    {
        PositionOrientation tankOrient = mPositionOrientations[mContentID];
        
        gl::pushMatrices();
        
        gl::setMatrices( mCam );

        gl::translate(tankOrient.position);

        gl::rotate(tankOrient.directionDegrees, 0, 1, 0);
        
        if (mIsGroundVisible)
        {
            GroundOrientaion groundOrient = mTankGroundOrientations[mContentID];
            
            // Adjust the height
            gl::translate(Vec3f(0,groundOrient.height,0));
            
            // Adjust the X angle
            gl::rotate(toDegrees(groundOrient.xAngleRads) - 90, 1, 0, 0);

            // Adjsut the Z angle
            gl::rotate(toDegrees(groundOrient.zAngleRads) - 90, 0, 0, 1);
            
        }

        renderPositionedTank();

        gl::popMatrices();
        
    }
    
    void TankContent::renderPositionedTank()
    {
        gl::enableAdditiveBlending();
        mTank->render();
        gl::disableAlphaBlending();
    }
    
    void TankContent::drawTankShots()
    {
        gl::enableAdditiveBlending();
        // Shots are in world coords
        gl::pushMatrices();
        gl::setMatrices( mCam );
        mTank->renderShots(mCam);
        gl::popMatrices();
        gl::disableAlphaBlending();
    }
    
    void TankContent::drawMinion()
    {
        // A simple spin around the tank.
        // TODO: Make this more interesting.
        
        gl::bindStockShader(gl::ShaderDef().color());

        gl::pushMatrices();
        gl::setMatrices( mCam );
        
        // Spin baby
        // TODO: Give this realistic movement
        gl::translate(mMinionPosition);
        gl::scale(Vec3f(kMinionScale,kMinionScale,kMinionScale));
        gl::color(1, 0, 0, mRenderAlpha);
        gl::setDefaultShaderVars();
        
        ColorAf randColor(CM_HSV,
                          Rand::randFloat(),
                          1.0f,
                          1.0f,
                          mRenderAlpha);

        mMinion->draw(Vec3f(sin(mNumFramesRendered * 0.1),
                            cos(mNumFramesRendered * 0.06666),
                            cos(mNumFramesRendered * 0.03333)), randColor);

        gl::popMatrices();
    }
    
    void TankContent::render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect)
    {
        mRenderAlpha = 1;
        mScreenAlpha = 1;
        
        if (mContentID == -1)
        {
            console() << "WARN: Tank content doesn't have a contentID\n";
        }
        
        // clear out both of the attachments of the FBO with black
        gl::clear( ColorAf( 0.0f, 0.0f, 0.0f, 0.0f ) );
        
        gl::disableDepthWrite();
        gl::disableDepthRead();
        gl::disableAlphaBlending();
        
        drawScreen(contentRect);

        gl::enableDepthWrite();
        gl::enableDepthRead();

        drawGround();
        
        drawTank();

        drawTankShots();
        
        drawMinion();
        
        // Clear out the content ID.
        // Do it through the getter.
        setFrameContentID(-1);
    }
}
