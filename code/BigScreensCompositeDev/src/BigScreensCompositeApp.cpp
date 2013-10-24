#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "MPEApp.hpp"
#include "MPEClient.h"
#include "Utilities.hpp"
#include "SharedTypes.hpp"
#include "TankContent.h"
#include "TextureContent.h"
#include "GridLayoutTimeline.h"
#include "SceneWindow.hpp"
#include "cinder/gl/GlslProg.h"
#include "OutLineBorder.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace mpe;
using namespace bigscreens;

#define IS_IAC  0

#if IS_IAC
const static float kScreenScale = 1.0f;
#else
const static float kScreenScale = 0.25f;
#endif

class BigScreensCompositeApp : public AppNative, public MPEApp, public ContentProvider
{
    
public:
    
    // Setup / Load
    void prepareSettings(Settings *settings);
	void setup();
    void shutdown();
    
    void reload();
    void loadAssets();
    void loadGrid();
    
    ci::DataSourceRef mpeSettingsFile();
    void mpeReset();
    
    // Input
    void keyUp(KeyEvent event);
    
    // Playback
    void advance();
    void reverse();
    void restart();
    void play();
    void pause();
    
    // Update
	void update();
    void mpeFrameUpdate(long serverFrameNumber);
	
    // Draw
    void draw();
    void mpeFrameRender(bool isNewFrame);
    void renderControls();
    
    // Messages
    void mpeMessageReceived(const std::string &message, const int fromClientID);
    
    // Content Provider
    RenderableContentRef contentForKey(const std::string & contentName);
    
    // Vars
    MPEClientRef mClient;
    
    GridLayoutTimelineRef   mTimeline;
    
    gl::TextureRef mTexturePlaying;
    gl::TextureRef mTexturePaused;
    
    // Content
    RenderableContentRef mTankContent0;
    float mTank0Rotation;
    RenderableContentRef mTankContent1;
    RenderableContentRef mTankContent2;

    
    RenderableContentRef mTextureContentBlank;
    OutLineBorderRef mOutLine;
  
//    gl::FboRef mFBO;
};

#pragma mark - Setup

void BigScreensCompositeApp::prepareSettings(Settings *settings)
{
#if IS_IAC
    settings->setFullScreen();
#endif
}

void BigScreensCompositeApp::setup()
{
    mClient = MPEClient::Create(this);
    mClient->setIsRendering3D(false);
    mClient->setIsScissorEnabled(false);
    
    GridLayoutTimeline *t = new GridLayoutTimeline(SharedGridAssetPath(!IS_IAC), kScreenScale);

    mTimeline = std::shared_ptr<GridLayoutTimeline>(t);
    
    mOutLine = std::shared_ptr<OutLineBorder>(new OutLineBorder());

    loadAssets();
    reload();
}

void BigScreensCompositeApp::shutdown()
{
}

#pragma mark - Load

void BigScreensCompositeApp::reload()
{
    mTimeline->reload();
    mTimeline->restart();
    
    static_pointer_cast<TankContent>(mTankContent0)->reset();
    static_pointer_cast<TankContent>(mTankContent1)->reset();
    static_pointer_cast<TankContent>(mTankContent2)->reset();
    mTank0Rotation = 0;
}

void BigScreensCompositeApp::loadAssets()
{
    // NOTE / TODO
    // These should share OBJs
    TankContent *tank0 = new TankContent();
    tank0->load("T72.obj");
    mTankContent0 = RenderableContentRef(tank0);

    TankContent *tank1 = new TankContent();
    tank1->load("T72.obj");
    mTankContent1 = RenderableContentRef(tank1);

    TankContent *tank2 = new TankContent();
    tank2->load("T72.obj");
    mTankContent2 = RenderableContentRef(tank2);

    TextureContent *texBlank = new TextureContent();
    texBlank->load("blank_texture.png");
    mTextureContentBlank = RenderableContentRef(texBlank);
    
    // Is there a more elegant way of doing this?    
    mTexturePlaying = gl::TextureRef(new gl::Texture(loadImage(app::loadResource("playing.png"))));
    mTexturePaused = gl::TextureRef(new gl::Texture(loadImage(app::loadResource("paused.png"))));
    
}

ci::DataSourceRef BigScreensCompositeApp::mpeSettingsFile()
{
# if IS_IAC
    string settingsFilename = "settings."+to_string(CLIENT_ID)+".IAC.xml";
#else
    string settingsFilename = "settings."+to_string(CLIENT_ID)+".xml";
#endif
    return ci::app::loadResource(settingsFilename);
}

void BigScreensCompositeApp::mpeReset()
{
    // Reset the state of your app.
    // This will be called when any client connects.
    reload();
}

#pragma mark - Content

RenderableContentRef BigScreensCompositeApp::contentForKey(const std::string & contentName)
{
    // It would be nice to only keep the content in memory when it's being used.
    // Perhaps this can lazy-load content...
    if (contentName == "tank")
    {
        return mTankContent0;
    }
    else if (contentName == "tank0")
    {
        return mTankContent1;
    }
    else if (contentName == "tank1")
    {
        return mTankContent2;
    }

    return mTextureContentBlank;

}

#pragma mark - Input events

const static std::string kMPEMessagePlay = "play";
const static std::string kMPEMessagePause = "pause";
const static std::string kMPEMessagePrev = "prev";
const static std::string kMPEMessageNext = "next";
const static std::string kMPEMessageLoad = "load";
const static std::string kMPEMessageRestart = "restart";

void BigScreensCompositeApp::keyUp(KeyEvent event)
{
    char key = event.getChar();
    if (key == ' ') // start / stop
    {
        if (mTimeline->isPlaying())
        {
            mClient->sendMessage(kMPEMessagePause);
        }
        else
        {
            mClient->sendMessage(kMPEMessagePlay);
        }
    }
    else if (event.getCode() == KeyEvent::KEY_RIGHT)
    {
        mClient->sendMessage(kMPEMessageNext);
    }
    else if (event.getCode() == KeyEvent::KEY_LEFT)
    {
        mClient->sendMessage(kMPEMessagePrev);
    }
    else if (key == 'r')
    {
        mClient->sendMessage(kMPEMessageRestart);
    }
    else if (key == 'l')
    {
        mClient->sendMessage(kMPEMessageLoad);
    }
}

#pragma mark - Playback

void BigScreensCompositeApp::restart()
{
    mTimeline->restart();
}

void BigScreensCompositeApp::play()
{
    mTimeline->play();
}

void BigScreensCompositeApp::pause()
{
    mTimeline->pause();
}

void BigScreensCompositeApp::advance()
{
    mTimeline->stepToNextLayout();
}

void BigScreensCompositeApp::reverse()
{
    mTimeline->stepToPreviousLayout();
}

#pragma mark - Update

void BigScreensCompositeApp::update()
{
    if (!mClient->isConnected() && (getElapsedFrames() % 60) == 0)
    {
        mClient->start();
    }
}

void BigScreensCompositeApp::mpeFrameUpdate(long serverFrameNumber)
{
    mTimeline->update();
    
    mTank0Rotation += 0.01;
    static_pointer_cast<TankContent>(mTankContent0)->update([=](CameraPersp & cam)
    {
        float camX = cosf(mTank0Rotation) * 1000;
        float camZ = sinf(mTank0Rotation) * 1000;
        cam.lookAt( Vec3f( camX, 400, camZ ), Vec3f( 0, 100, 0 ) );
    });

    float tank1Distance = sinf(mClient->getCurrentRenderFrame() * 0.0025);
    static_pointer_cast<TankContent>(mTankContent1)->update([=](CameraPersp & cam)
    {
        // Zoom in and out
        float camZ = tank1Distance * 500;
        cam.lookAt(Vec3f( 100, 500, camZ ),
                   Vec3f( 0, 100, 0 ) );
    });
    
    float tank2Bounce = cosf((mClient->getCurrentRenderFrame() + ((arc4random() % 8) - 4)) * 0.5);
    static_pointer_cast<TankContent>(mTankContent2)->update([=](CameraPersp & cam)
    {
        // Bouncy shot
        cam.lookAt(Vec3f( 0, 800 + (tank2Bounce * 10), -1000 ),
                   Vec3f( 0, 100, 0 ) );
    });

}

#pragma mark - Render

void BigScreensCompositeApp::draw()
{
    mClient->draw();
}

void BigScreensCompositeApp::mpeFrameRender(bool isNewFrame)
{
    gl::clear( Color( 0, 0, 0 ), true );
    
    std::vector<std::pair<Rectf, RenderableContentRef> > renderContent =
        mTimeline->getRenderContent(this);
    
    Vec2i windowSize = mClient->getVisibleRect().getSize();
    Vec2i offset = mClient->getVisibleRect().getUpperLeft();
    
//    gl::enableDepthRead();
//    gl::enableDepthWrite();
//    gl::disableDepthRead();
//    gl::disableDepthWrite();

    for (int i = 0; i < renderContent.size(); ++i)
    {
        // NOTE: Maybe we should load up the scene windows whenever the layout
        // changes and just update their rects in update().
        // We could avoid allocating new memory for each scene every frame.
        
        std::pair<Rectf, RenderableContentRef> & renderMe = renderContent[i];
        
        // TODO: Check if this should be rendered at all.
        // Maybe that happens in scenewindow
        if (rectsOverlap(renderMe.first, mClient->getVisibleRect()))
        {
            SceneWindow scene(renderMe.second,
                              renderMe.first,
                              windowSize);
    //                          mFBO);
            scene.render(offset);

            // NOTE:
            // How exactly does this work?
            mOutLine->render();
        }
    }

    if (CLIENT_ID == 0)
    {
        renderControls();
    }
}

void BigScreensCompositeApp::renderControls()
{
    // Draw the controls

    // NOTE: This breaks the rendering of screens 2 & 3, so only draw controls on screen 1
    
    gl::viewport(0, 0, getWindowWidth(), getWindowHeight());
    gl::setMatricesWindow( getWindowSize() );
    Vec2i offset = mClient->getVisibleRect().getUpperLeft();
    gl::translate(offset);
    
    gl::enableAlphaBlending();
    gl::color(Color::white());
    if (mTimeline->isPlaying())
    {
        gl::draw(mTexturePlaying, Rectf(15,15,35,35));
    }
    else
    {
        gl::draw(mTexturePaused, Rectf(15,15,35,35));
    }
    
    gl::disableAlphaBlending();

}

#pragma mark - Messages

void BigScreensCompositeApp::mpeMessageReceived(const std::string &message, const int fromClientID)
{
    if (message == kMPEMessagePlay)
    {
        play();
    }
    else if (message == kMPEMessagePause)
    {
        pause();
    }
    else if (message == kMPEMessageNext)
    {
        advance();
    }
    else if (message == kMPEMessagePrev)
    {
        reverse();
    }
    else if (message == kMPEMessageRestart)
    {
        restart();
    }
    else if (message == kMPEMessageLoad)
    {
        reload();
    }
}

#pragma mark - Misc

CINDER_APP_NATIVE( BigScreensCompositeApp, RendererGl )
