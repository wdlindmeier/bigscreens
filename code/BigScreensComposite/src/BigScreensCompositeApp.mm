#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "MPEApp.hpp"
#include "MPEClient.h"
#include "GridLayout.h"
#include "SharedTypes.hpp"
#include "Utilities.hpp"
#include "TankContent.h"

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

class BigScreensCompositeApp : public AppNative, public MPEApp
{
    
public:
    
    // Setup / Load
    void prepareSettings(Settings *settings);
	void setup();
    
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
    void newLayoutWasSet();
    
    // Update
	void update();
    void mpeFrameUpdate(long serverFrameNumber);
	
    // Draw
    void draw();
    void mpeFrameRender(bool isNewFrame);
    
    void calculateTotalDuration();
    
    // Vars
    
    MPEClientRef mClient;

    vector<GridLayout> mGridLayouts;
    int mIdxCurrentLayout;
    int mIdxPrevLayout;
    
    float mTransitionAmt;
    long long mStartTime;
    long long mPlayheadTime;
    long long mLastFrameTime;
    long long mTotalDuration;
    float mPlaybackSpeed;
    bool mIsPlaying;

    gl::Texture mScreenTexture;
    gl::Texture mTexturePlaying;
    gl::Texture mTexturePaused;
    
    TankContent mContent;

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
    loadAssets();
    reload();
}

#pragma mark - Load

void BigScreensCompositeApp::reload()
{
    mIdxCurrentLayout = -1;
    mIdxPrevLayout = -1;
    mTransitionAmt = 1.0f;
    mPlaybackSpeed = 1.0f;
    mTotalDuration = 0;
    mStartTime = 0;
    mPlayheadTime = 0;
    mLastFrameTime = getMilliCount();
    mContent.reset();
    loadGrid();
}

void BigScreensCompositeApp::loadAssets()
{
    mScreenTexture = loadImage(app::loadResource("screen.png"));
    mTexturePlaying = loadImage(app::loadResource("playing.png"));
    mTexturePaused = loadImage(app::loadResource("paused.png"));
    
    mContent.load("T72.obj");
}

void BigScreensCompositeApp::loadGrid()
{
    mGridLayouts.clear();
    fs::path gridPath = SharedGridPath();
    mGridLayouts = GridLayout::loadAllFromPath(gridPath, kScreenScale);
    
    if (mGridLayouts.size() == 0)
    {
        console() << "Didn't find any serialized grids." << endl;
        // Add an empty layout
        GridLayout newLayout;
        newLayout.setTimestamp(0);
        newLayout.setTransitionDuration(kDefaultTransitionDuration);
        mGridLayouts.push_back(newLayout);
    }
    else
    {
        console() << mGridLayouts.size() << " Layouts found\n";
    }
    
    mIdxCurrentLayout = 0;
    
    calculateTotalDuration();
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

#pragma mark - Input events

void BigScreensCompositeApp::keyUp(KeyEvent event)
{
    char key = event.getChar();
    if (key == ' ') // start / stop
    {
        if (mIsPlaying)
        {
            pause();
        }
        else
        {
            play();
        }
    }
    else if (event.getCode() == KeyEvent::KEY_RIGHT)
    {
        advance();
    }
    else if (event.getCode() == KeyEvent::KEY_LEFT)
    {
        reverse();
    }
    else if (key == 'r')
    {
        restart();
    }
    else if (key == 'l')
    {
        reload();
    }
}

#pragma mark - Playback

void BigScreensCompositeApp::restart()
{
    mStartTime = 0;
    mIdxCurrentLayout = 0;
    mTransitionAmt = 1.0f;
    mPlayheadTime = 0;
}

void BigScreensCompositeApp::play()
{
    mLastFrameTime = getMilliCount();
    mIsPlaying = true;
}

void BigScreensCompositeApp::pause()
{
    mIsPlaying = false;
}

void BigScreensCompositeApp::advance()
{
    mIdxPrevLayout = mIdxCurrentLayout;
    mIdxCurrentLayout = (mIdxCurrentLayout + 1) % mGridLayouts.size();
    newLayoutWasSet();
}

void BigScreensCompositeApp::reverse()
{
    mIdxPrevLayout = mIdxCurrentLayout;
    mIdxCurrentLayout = mIdxCurrentLayout - 1;
    if (mIdxCurrentLayout < 0)
    {
        mIdxCurrentLayout = mGridLayouts.size() - 1;
    }
    
    newLayoutWasSet();
}

void BigScreensCompositeApp::newLayoutWasSet()
{
    mTransitionAmt = mIsPlaying ? 0.0f : 1.0f;
    
    // Always reset the playhead so if we fwd/bck while playing it
    // continues from the current frame.
    GridLayout & layout = mGridLayouts[mIdxCurrentLayout];
    mPlayheadTime = layout.getTimestamp();
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
    if (mIsPlaying)
    {
        long long timestamp = getMilliCount();
        
        long long timeDelta = (timestamp - mLastFrameTime) * mPlaybackSpeed;
        mPlayheadTime += timeDelta;
        
        mLastFrameTime = timestamp;
        
        int nextID = mIdxCurrentLayout + 1;
        
        // Don't loop. We can do this manually
        if (nextID < mGridLayouts.size())
        {
            GridLayout & nextLayout = mGridLayouts[nextID];
            
            long long startTimeNextLayout = nextLayout.getTimestamp();
            if (startTimeNextLayout <= mPlayheadTime)
            {
                advance();
            }
        }
        
        GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
        long long startTimeCurLayout = curLayout.getTimestamp();
        long timeIntoLayout = mPlayheadTime - startTimeCurLayout;
        long transitionDuration = curLayout.getTransitionDuration();
        
        // Dont transition into the first slide
        if (mIdxCurrentLayout > 0)
        {
            mTransitionAmt = (float)std::min<double>((double)timeIntoLayout / (double)transitionDuration, 1.0);
        }
    }
    
    mContent.update();
    
}

#pragma mark - Render

void BigScreensCompositeApp::draw()
{
    mClient->draw();
}

void BigScreensCompositeApp::mpeFrameRender(bool isNewFrame)
{
    mContent.render();
    
    gl::clear( Color( 0, 0, 0 ), true );

    // set the viewport to match our window
    gl::setViewport( Area(mClient->getVisibleRect()) );
    gl::setMatricesWindow( getWindowSize() );
    
    gl::enableAlphaBlending();
    
    if (mIdxCurrentLayout > -1)
    {
        GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
        
        GridRenderMode mode = GridRenderModeNormal;

        // TODO: Only render the views that are on-screen
        if (mTransitionAmt < 1.0 && mIdxPrevLayout != -1)
        {
            GridLayout & prevLayout = mGridLayouts[mIdxPrevLayout];
            
            prevLayout.render(1.0 - std::min(mTransitionAmt / 0.5f, 1.0f),
                              curLayout,
                              Vec2f::zero(),
                              mode,
                              mContent);
            
            curLayout.render(std::max((mTransitionAmt - 0.5f) / 0.5f, 0.0f),
                             prevLayout,
                             Vec2f::zero(),
                             mode,
                             mContent);
            
        }
        else
        {
            curLayout.render(mTransitionAmt,
                             GridLayout(),
                             Vec2f::zero(),
                             mode,
                             mContent);
        }
    }
    
    gl::color(Color::white());
    if (mIsPlaying)
    {
        gl::draw(mTexturePlaying, Rectf(15,15,35,35));
    }
    else
    {
        gl::draw(mTexturePaused, Rectf(15,15,35,35));
    }
    
    // Draw the current frame num
    gl::drawString("Frame " + to_string(mIdxCurrentLayout), Vec2f(42, 25));

}

#pragma mark - Misc

void BigScreensCompositeApp::calculateTotalDuration()
{
    int numLayouts = mGridLayouts.size();
    GridLayout & lastLayout = mGridLayouts[numLayouts - 1];
    mTotalDuration = lastLayout.getTimestamp() + lastLayout.getTransitionDuration();
}


// If you're deploying to iOS, set the Render antialiasing to 0 for a significant
// performance improvement. This value defaults to 4 (AA_MSAA_4) on iOS and 16 (AA_MSAA_16)
// on the Desktop.
#if defined( CINDER_COCOA_TOUCH )
CINDER_APP_NATIVE( BigScreensCompositeApp, RendererGl(RendererGl::AA_NONE) )
#else
CINDER_APP_NATIVE( BigScreensCompositeApp, RendererGl )
#endif
