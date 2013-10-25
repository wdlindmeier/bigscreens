#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"
#include "cinder/Text.h"
#include "MPEApp.hpp"
#include "MPEClient.h"
#include "Utilities.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace mpe;
using namespace bigscreens;

class BigScreensControllerApp : public AppNative, public MPEApp
{
public:
	void setup();
    ci::DataSourceRef mpeSettingsFile();
    void mpeReset();
    
	void update();
    void mpeFrameUpdate(long serverFrameNumber);
	
    void draw();
    void mpeFrameRender(bool isNewFrame);
    void renderControls();
    void renderCurrentFrameTexture();
    
    void mpeMessageReceived(const std::string &message, const int fromClientID);
    
    void mouseUp(MouseEvent event);
    
    MPEClientRef mClient;
    
    gl::TextureRef mTexturePlay;
    gl::TextureRef mTexturePause;
    gl::TextureRef mTextureShowColumns;
    gl::TextureRef mTextureHideColumns;
    gl::TextureRef mTextureCurrentFrame;
    
    bool mIsDrawingColumns;
    bool mIsPlaying;
    
    Rectf mRectPlaybackButton;
    Rectf mRectColumnButton;
    
    int mCurrentLayoutIndex;
    bool mShouldRenderFrameNum;

};

#pragma mark - Setup

void BigScreensControllerApp::setup()
{
    mClient = MPEClient::Create(this);
    
    mTexturePlay = gl::TextureRef(new gl::Texture(loadImage(app::loadResource("play.png"))));
    mTexturePause = gl::TextureRef(new gl::Texture(loadImage(app::loadResource("pause.png"))));
    
    mTextureShowColumns = gl::TextureRef(new gl::Texture(loadImage(app::loadResource("show_columns.png"))));
    mTextureHideColumns = gl::TextureRef(new gl::Texture(loadImage(app::loadResource("hide_columns.png"))));
    
    mIsDrawingColumns = false;
    mIsPlaying = false;
    
    mCurrentLayoutIndex = -1;    
    mShouldRenderFrameNum = true;
    
    mRectPlaybackButton = Rectf(15, 15, 55, 55);
    mRectColumnButton = Rectf(70, 15, 208, 55);
}

ci::DataSourceRef BigScreensControllerApp::mpeSettingsFile()
{
    return ci::app::loadResource("settings."+to_string(CLIENT_ID)+".xml");
}

void BigScreensControllerApp::mpeReset()
{
    // IMPORTANT: Keep this state in-sync with the app
    mIsPlaying = false;
    mIsDrawingColumns = false;
}

#pragma mark - Update

void BigScreensControllerApp::update()
{
    if (!mClient->isConnected() && (getElapsedFrames() % 60) == 0)
    {
        mClient->start();
    }
}

void BigScreensControllerApp::mpeFrameUpdate(long serverFrameNumber)
{
    // Your update code.
}

#pragma mark - Drawing

void BigScreensControllerApp::renderCurrentFrameTexture()
{
    fs::path assetPath = SharedAssetPath(!IS_IAC) / "Hyperspace.ttf";
    if (fs::exists(assetPath))
    {
        Font font(loadFile(assetPath), 24);
        Surface frameSurf = renderString("Layout: " + std::to_string(mCurrentLayoutIndex),
                                         font,
                                         Color::black());
        gl::Texture *frameTex = new gl::Texture(frameSurf);
        mTextureCurrentFrame = gl::TextureRef(frameTex);
    }
    else
    {
        console() << "ERROR: asset doesnt exist: " << assetPath << endl;
    }
    mShouldRenderFrameNum = false;
}

void BigScreensControllerApp::draw()
{
    if (mShouldRenderFrameNum)
    {
        renderCurrentFrameTexture();
    }
    mClient->draw();
}

void BigScreensControllerApp::mpeFrameRender(bool isNewFrame)
{
    gl::clear(Color(0.5,0.5,0.5));
    // Your render code.
    renderControls();
}

void BigScreensControllerApp::renderControls()
{
    gl::viewport(0, 0, getWindowWidth(), getWindowHeight());
    gl::setMatricesWindow( getWindowSize() );
 
    gl::enableAlphaBlending();
    gl::color(Color::white());
    if (mIsPlaying)
    {
        gl::draw(mTexturePause, mRectPlaybackButton);
    }
    else
    {
        gl::draw(mTexturePlay, mRectPlaybackButton);
    }
    
    if (mIsDrawingColumns)
    {
        gl::draw(mTextureHideColumns, mRectColumnButton);
    }
    else
    {
        gl::draw(mTextureShowColumns, mRectColumnButton);
    }
    
    // Draw the frame num
    if (mTextureCurrentFrame)
    {
        gl::draw(mTextureCurrentFrame, Rectf(20,60,
                                             20 + mTextureCurrentFrame->getWidth(),
                                             60 + mTextureCurrentFrame->getHeight()));
    }
    
    gl::disableAlphaBlending();
}

#pragma mark - Message

void BigScreensControllerApp::mpeMessageReceived(const std::string &message, const int fromClientID)
{
    vector<string> tokens = ci::split(message, kMPEMessageDelimeter);
    string command = tokens[0];
    
    if (command == kMPEMessageShowColumns)
    {
        mIsDrawingColumns = true;
    }
    else if (command == kMPEMessageHideColumns)
    {
        mIsDrawingColumns = false;
    }
    else if (command == kMPEMessagePlay)
    {
        mIsPlaying = true;
    }
    else if (command == kMPEMessagePause)
    {
        mIsPlaying = false;
    }
    else if (command == kMPEMessageCurrentLayout)
    {
        mCurrentLayoutIndex = stoi(tokens[1]);
        mShouldRenderFrameNum = true;
    }
}

#pragma mark - Input

void BigScreensControllerApp::mouseUp(cinder::app::MouseEvent event)
{
    Vec2i eventPos = event.getPos();
    if (mClient->isConnected())
    {
        if (mRectColumnButton.contains(eventPos))
        {
            if (mIsDrawingColumns)
            {
                mClient->sendMessage(kMPEMessageHideColumns);
            }
            else
            {
                mClient->sendMessage(kMPEMessageShowColumns);
            }
            
        }
        else if (mRectPlaybackButton.contains(eventPos))
        {
            if (mIsPlaying)
            {
                mClient->sendMessage(kMPEMessagePause);
            }
            else
            {
                mClient->sendMessage(kMPEMessagePlay);
            }
        }
    }
}

// If you're deploying to iOS, set the Render antialiasing to 0 for a significant
// performance improvement. This value defaults to 4 (AA_MSAA_4) on iOS and 16 (AA_MSAA_16)
// on the Desktop.
#if defined( CINDER_COCOA_TOUCH )
CINDER_APP_NATIVE( BigScreensControllerApp, RendererGl(RendererGl::AA_NONE) )
#else
CINDER_APP_NATIVE( BigScreensControllerApp, RendererGl )
#endif
