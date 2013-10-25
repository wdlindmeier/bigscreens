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
#include "cinder/gl/Shader.h"
#include "OutLineBorder.hpp"
#include "TankHeightmapContent.h"
#include "PerlinContent.h"
#include "cinder/Rand.h"
#include "cinder/qtime/QuickTimeGl.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace mpe;
using namespace bigscreens;

class BigScreensCompositeApp : public AppNative, public MPEApp, public ContentProvider
{
    
public:
    
    // Setup
    void prepareSettings(Settings *settings);
	void setup();
    void shutdown();
    
    // Load
    void reload();
    void loadAssets();
    void loadGrid();
    void loadAudio();
    
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
    void updatePlaybackState();
	
    // Draw
    void draw();
    void mpeFrameRender(bool isNewFrame);
    void renderColumns();
    
    // Messages
    void mpeMessageReceived(const std::string &message, const int fromClientID);
    void broadcastCurrentLayout();
    
    // Content Provider
    RenderableContentRef contentForKey(const std::string & contentName);
    
    // Vars
    MPEClientRef mClient;
    
    GridLayoutTimelineRef mTimeline;
    
    bool mIsDrawingColumns;
    int mLastFrameNum;
    
    // Audio
    qtime::MovieGlRef    mSoundtrack;
    
    // Content
    RenderableContentRef mTankContent0;
    float mTank0Rotation;
    RenderableContentRef mTankContent1;
    RenderableContentRef mTankContent2;
    RenderableContentRef mTankContentHeightmap;
    RenderableContentRef mPerlinContent;
    
    RenderableContentRef mTextureContentBlank;
    OutLineBorderRef mOutLine;

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
    
    console() << "IS_IAC ? " << IS_IAC << "\n";
    GridLayoutTimeline *t = new GridLayoutTimeline(SharedGridAssetPath(!IS_IAC), kScreenScale);

    mTimeline = std::shared_ptr<GridLayoutTimeline>(t);
    
    mOutLine = std::shared_ptr<OutLineBorder>(new OutLineBorder());
    
    mIsDrawingColumns = false;

    loadAudio();
    loadAssets();
    reload();
}

void BigScreensCompositeApp::shutdown()
{
}

#pragma mark - Load

void BigScreensCompositeApp::reload()
{
    mLastFrameNum = -1;
    mTimeline->reload();
    mTimeline->restart();
    mSoundtrack->seekToStart();
    
    static_pointer_cast<TankContent>(mTankContent0)->reset();
    static_pointer_cast<TankContent>(mTankContent1)->reset();
    static_pointer_cast<TankContent>(mTankContent2)->reset();
    static_pointer_cast<TankContent>(mTankContentHeightmap)->reset();
    static_pointer_cast<PerlinContent>(mPerlinContent)->reset();
    
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
    
    TankHeightmapContent *tankHeightmap = new TankHeightmapContent();
    tankHeightmap->load("T72.obj");
    mTankContentHeightmap = RenderableContentRef(tankHeightmap);
    
    PerlinContent *perlinContent = new PerlinContent();
    mPerlinContent = RenderableContentRef(perlinContent);

    TextureContent *texBlank = new TextureContent();
    texBlank->load("blank_texture.png");
    mTextureContentBlank = RenderableContentRef(texBlank);
}

void BigScreensCompositeApp::loadAudio()
{
    fs::path audioPath = SharedAssetPath(!IS_IAC) / "audio" / "escape_from_ny_theme.mp3"; //getResourcePath("escape_from_ny_theme.mp3");
    if (!fs::exists(audioPath))
    {
        console() << "ERROR: No audio path found\n";
    }
    else
    {
        console() << "Audio path: " << audioPath << endl;
    }
    mSoundtrack = qtime::MovieGl::create(audioPath);
    console() << "mSoundtrack: " << mSoundtrack << endl;
    mSoundtrack->setLoop(!IS_IAC);
    try
    {
        mSoundtrack->setupMonoFft( kNumFFTChannels );
    }
    catch( qtime::QuickTimeExcFft & )
    {
        console() << "Unable to setup FFT" << std::endl;
    }
    console() << "FFT Channels: " << mSoundtrack->getNumFftChannels() << std::endl;
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
    // IMPORTANT: Keep this state in-sync with the controller
    mTimeline->pause();
    mIsDrawingColumns = false;
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
        return mTankContentHeightmap;
    }
    else if (contentName == "perlin")
    {
        return mPerlinContent;
    }

    return mTextureContentBlank;

}

#pragma mark - Input events

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
    console() << "restart\n";
    mTimeline->restart();
    mSoundtrack->seekToStart();
    if (mTimeline->isPlaying())
    {
        mSoundtrack->play();
    }
}

void BigScreensCompositeApp::play()
{
    console() << "play\n";
    mTimeline->play();
    mSoundtrack->play();
}

void BigScreensCompositeApp::pause()
{
    console() << "pause\n";
    mTimeline->pause();
    mSoundtrack->stop();
}

void BigScreensCompositeApp::advance()
{
    console() << "advance\n";
    mTimeline->stepToNextLayout();
}

void BigScreensCompositeApp::reverse()
{
    console() << "reverse\n";
    mTimeline->stepToPreviousLayout();
}

#pragma mark - Update

void BigScreensCompositeApp::update()
{
    if (!mClient->isConnected() && (getElapsedFrames() % 60) == 0)
    {
        mClient->start();
    }
    //updatePlaybackState();
}

void BigScreensCompositeApp::mpeFrameUpdate(long serverFrameNumber)
{
    mTimeline->update();
    
    // FFT Data
    // float *fftData = mSoundtrack->getFftData();
    
    // Send the controller the current frame if it's changed
    if (mLastFrameNum != mTimeline->getCurrentFrame())
    {
        broadcastCurrentLayout();
        mLastFrameNum = mTimeline->getCurrentFrame();
    }
    
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

    static_pointer_cast<TankHeightmapContent>(mTankContentHeightmap)->update([=](CameraPersp & cam)
    {
        // Steady shot
        cam.lookAt(Vec3f( 0, 600, -1000 ),
                   Vec3f( 0, 100, 0 ) );
    });
    
    // Move up 1 px. Maybe this is too fast
    static_pointer_cast<PerlinContent>(mPerlinContent)->update(Vec2f(0, 1));
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
        
        // Check if this should be rendered at all.
        // Maybe this should happen in SceneWindow?
        if (rectsOverlap(renderMe.first, mClient->getVisibleRect()))
        {
            SceneWindow scene(renderMe.second,
                              renderMe.first,
                              windowSize);
            scene.render(offset);

            // NOTE:
            // How exactly does this work?
            mOutLine->render();
        }
    }
    
    if (mIsDrawingColumns)
    {
        renderColumns();
    }
}

void BigScreensCompositeApp::renderColumns()
{
    gl::viewport(0, 0, getWindowWidth(), getWindowHeight());
    gl::setMatricesWindow( getWindowSize() );
    gl::translate(mClient->getVisibleRect().getUpperLeft() * -1);

    gl::bindStockShader(gl::ShaderDef().color());
    gl::color(ColorAf(0.95,0.95,0.9));
    
    // Column 1
    float columnHeight = getWindowHeight();
    float x = kPosColumn1 * kScreenScale;
    float x1 = x - (kColumnWidth * 0.5f * kScreenScale);
    float x2 = x + (kColumnWidth * 0.5f * kScreenScale);
    float y1 = 0;
    float y2 = columnHeight;
    gl::drawSolidRect(Rectf(x1,y1,x2,y2));
    
    // Column 2
    x = kPosColumn2 * kScreenScale;
    x1 = x - (kColumnWidth * 0.5f * kScreenScale);
    x2 = x + (kColumnWidth * 0.5f * kScreenScale);
    gl::drawSolidRect(Rectf(x1,y1,x2,y2));
    
    // Column 3
    x = kPosColumn3 * kScreenScale;
    x1 = x - (kColumnWidth * 0.5f * kScreenScale);
    x2 = x + (kColumnWidth * 0.5f * kScreenScale);
    gl::drawSolidRect(Rectf(x1,y1,x2,y2));
    
    // Column 4
    x = kPosColumn4 * kScreenScale;
    x1 = x - (kColumnWidth * 0.5f * kScreenScale);
    x2 = x + (kColumnWidth * 0.5f * kScreenScale);
    gl::drawSolidRect(Rectf(x1,y1,x2,y2));
}

#pragma mark - Messages

void BigScreensCompositeApp::mpeMessageReceived(const std::string &message, const int fromClientID)
{
    // NOTE: These should always happen on the main thread
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
    else if (message == kMPEMessageShowColumns)
    {
        mIsDrawingColumns = true;
    }
    else if (message == kMPEMessageHideColumns)
    {
        mIsDrawingColumns = false;
    }
}

void BigScreensCompositeApp::broadcastCurrentLayout()
{
    // We want the frame to be sent even if there's only 1 client connected.
    // That means it will be sent more than once when more than 1 are rendering.
    // if (CLIENT_ID == 1)
    {
        // Send the current frame to the controller
        vector<int> toClients;
        toClients.push_back(555);
        mClient->sendMessage(kMPEMessageCurrentLayout +
                             kMPEMessageDelimeter +
                             std::to_string(mTimeline->getCurrentFrame()),
                             toClients);
    }
}

#pragma mark - Misc

CINDER_APP_NATIVE( BigScreensCompositeApp, RendererGl )
