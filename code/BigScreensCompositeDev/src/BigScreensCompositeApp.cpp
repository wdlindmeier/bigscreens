#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "MPEApp.hpp"
#include "MPEClient.h"
#include "Utilities.hpp"
#include "SharedTypes.hpp"
#include "TankContent.h"
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
    
    RenderableContentRef mTankContent;
    OutLineBorderRef mOutLine;
    
};

#pragma mark - Setup

void BigScreensCompositeApp::prepareSettings(Settings *settings)
{
#if IS_IAC
    settings->setBorderless();
#endif
}

void BigScreensCompositeApp::setup()
{
    mClient = MPEClient::Create(this);
    mClient->setIsRendering3D(false);
    mClient->setIsScissorEnabled(false);
    
    GridLayoutTimeline *t = new GridLayoutTimeline(SharedGridAssetPath(IS_IAC), kScreenScale);
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
}

void BigScreensCompositeApp::loadAssets()
{
    TankContent *tank = new TankContent();
    tank->load("T72.obj");
    mTankContent = RenderableContentRef(tank);
    
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
    // TMP: We'll always return the tank for now, since there's nothing else to return.
    // This could be a map, but we don't necessarily want to keep all of the content in memory...
    // We'll wait and see.
    return mTankContent;
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
    static_pointer_cast<TankContent>(mTankContent)->update();
    mTimeline->update();
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
    
    for (int i = 0; i < renderContent.size(); ++i)
    {
        // NOTE: Maybe we should load up the scene windows whenever the layout
        // changes and just update their rects in update().
        // We could avoid allocating new memory for each scene every frame.
        
        std::pair<Rectf, RenderableContentRef> & renderMe = renderContent[i];
        
        // TODO: Check if this should be rendered at all.
        // Maybe that happens in scenewindow
        
        SceneWindow scene(renderMe.second,
                          renderMe.first,
                          windowSize);
        scene.render(offset);
        mOutLine->render();
    }

    renderControls();
    
}

void BigScreensCompositeApp::renderControls()
{
    // Draw the controls
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
