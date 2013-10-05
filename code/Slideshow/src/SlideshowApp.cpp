#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "MPEApp.hpp"
#include "MPEClient.h"
#include "cinder/params/Params.h"
#include "cinder/Utilities.h"
#include <boost/iterator/filter_iterator.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace mpe;

// NEXT STEP: Create multiple targets
// https://github.com/wdlindmeier/Most-Pixels-Ever-Cinder/wiki/MPE-Setup-Tutorial-for-Cinder#3-create-multiple-targets

class SlideshowApp : public AppNative, public MPEApp
{
public:
	void setup();
    void mpeReset();
    void prepareSettings(Settings *settings);
	void update();
    void updateParams();
    void mpeFrameUpdate(long serverFrameNumber);

    void updateCurrentSlide();
    void loadAllSlides();
    void preloadNextSlide();
	
    void draw();
    void mpeFrameRender(bool isNewFrame);
    
    void mpeMessageReceived(const std::string & message, const int fromClientID);

    ci::DataSourceRef mpeSettingsFile();
    
    void keyDown(KeyEvent event);
    
    MPEClientRef mClient;
    
    fs::path mSlidesDirectory;

    int mSlideRate;
    bool mIsPaused;
    int mPrevSlideRate;
    bool mPrevIsPaused;

    int mCurrentSlide;
    int mNumSlides;
    int mFrameOffset;
    bool mDidUpdate;
    bool mShouldAdvance;
    bool mShouldReverse;
    
    params::InterfaceGlRef mParams;
    
    vector<fs::path>  mSlidePaths;
    
    gl::Texture mSlideTexture;
    gl::Texture mNextSlideTexture;
    
};

ci::DataSourceRef SlideshowApp::mpeSettingsFile()
{
    return ci::app::loadAsset("settings."+to_string(CLIENT_ID)+".xml");
}

const static int kInitialSlideRate = 120;

void SlideshowApp::prepareSettings(Settings *settings)
{
    settings->setBorderless();
}

void SlideshowApp::setup()
{
    mClient = MPEClient::Create(this);
    mClient->setIsRendering3D(false);
    
    mSlideRate = kInitialSlideRate;
    mIsPaused = false;
    mPrevSlideRate = mSlideRate;
    mPrevIsPaused = mIsPaused;
    
    mNumSlides = 0;
    mCurrentSlide = 0;
    mFrameOffset = 0;
    mDidUpdate = false;
    
    mShouldAdvance = false;
    mShouldReverse = false;
    
    mParams = params::InterfaceGl::create("Slideshow", Vec2f(200, 50));
    mParams->addParam("Rate (frames)", &mSlideRate, "min=1");
    mParams->addParam("Pause", &mIsPaused);
    
    mSlidesDirectory = getAppPath() / ".." / "Slides";
    
    if ( !fs::exists( mSlidesDirectory ) )
    {
        fs::create_directories( mSlidesDirectory );
    }
    
    if (!mClient->isAsynchronousClient())
    {
        loadAllSlides();
        // preloadNextSlide();
    }
}

void SlideshowApp::loadAllSlides()
{
    if ( fs::exists(mSlidesDirectory) && fs::is_directory(mSlidesDirectory) )
    {
        console() << "Found the input path: " << mSlidesDirectory << std::endl;
    }
    else
    {
        console() << "ERROR: Couldn't find the input directory: " << mSlidesDirectory << std::endl;
        return;
    }
    
    fs::directory_iterator dir_first(mSlidesDirectory), dir_last;
    
    // Filter images
    auto pred = [](const fs::directory_entry& p)
    {
        string filename = string(p.path().filename().c_str());
        std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
        return fs::is_regular_file(p) &&
        (filename.find( ".png" ) != -1 ||
         filename.find( ".gif" ) != -1 ||
         filename.find( ".jpeg" ) != -1 ||
         filename.find( ".gif" ) != -1 ||
         filename.find( ".jpg" ) != -1);
    };
    
    std::copy(make_filter_iterator(pred, dir_first, dir_last),
              make_filter_iterator(pred, dir_last, dir_last),
              std::back_inserter(mSlidePaths));
    
    mNumSlides = mSlidePaths.size();
    console() << "Found " << mNumSlides << " slides." << endl;
}

void SlideshowApp::mpeReset()
{
    // Reset the state of your app.
    // This will be called when any client connects.
    mCurrentSlide = 0;
    mFrameOffset = 0;
    mIsPaused = false;
    mSlideRate = kInitialSlideRate;
    // This will force a preload
    // We dont want to load any gl stuff on mpe calls.
    mDidUpdate = true;
}

void SlideshowApp::preloadNextSlide()
{
    fs::path slidePath = mSlidePaths[mCurrentSlide];
    Surface slideSurf = loadImage( loadFile( slidePath ) );
    Vec2i slideSize = slideSurf.getSize();
    Rectf clientRect = mClient->getVisibleRect();
    
    int cropX = std::min<int>(clientRect.x1, slideSize.x);
    int cropY = std::min<int>(clientRect.y1, slideSize.y);
    
    int cropWidth = std::min<int>(std::max<int>(slideSize.x - cropX, 0),
                                  clientRect.getWidth());
    
    int cropHeight = std::min<int>(std::max<int>(slideSize.y - cropY, 0),
                                   clientRect.getHeight());
    
    Surface cropped(cropWidth,cropHeight,slideSurf.hasAlpha());
    
    Area cropArea(Vec2i(cropX, cropY), Vec2i(cropX + cropWidth, cropY + cropHeight));
    
    cropped.copyFrom(slideSurf, cropArea, cropArea.getUL() * -1);
    
    // Clear it out
    slideSurf = Surface(0,0,0);
    
    mNextSlideTexture = cropped;
}

void SlideshowApp::updateCurrentSlide()
{
    if (mNextSlideTexture)
    {
        mDidUpdate = true;
        mSlideTexture = mNextSlideTexture;
        // Create an offset so every slide gets it's fully alotted time
        mFrameOffset = mClient->getCurrentRenderFrame() % mSlideRate;
    }
}

void SlideshowApp::updateParams()
{
    if (mSlideRate != mPrevSlideRate)
    {
        mClient->sendMessage("SLIDERATE#" + to_string(mSlideRate));
        mPrevSlideRate = mSlideRate;
    }
    if (mPrevIsPaused != mIsPaused)
    {
        mClient->sendMessage("PAUSE#" + to_string(mIsPaused));
        mPrevIsPaused = mIsPaused;
    }
}

void SlideshowApp::update()
{
    if (!mClient->isConnected() && (getElapsedFrames() % 60) == 0)
    {
        mClient->start();
    }
    
    if (mClient->isAsynchronousClient())
    {
        updateParams();
    }
    else
    {
        if (mDidUpdate)
        {
            preloadNextSlide();
            mDidUpdate = false;
        }
        else if (mShouldAdvance == true)
        {
            if (mNumSlides > 0)
            {
                mCurrentSlide = (mCurrentSlide + 1) % mNumSlides;
                updateCurrentSlide();
            }
            mShouldAdvance = false;
        }
        else if (mShouldReverse == true)
        {
            if (mNumSlides > 0)
            {
                mCurrentSlide = mCurrentSlide - 1;
                if (mCurrentSlide < 0)
                {
                    mCurrentSlide = mNumSlides - 1;
                }
                updateCurrentSlide();
            }
            mShouldReverse = false;
        }
        else
        {
            // NOTE: We're doing this in the main thread (e.g. not mpe update) to avoid threading issues
            if (!mIsPaused &&
                mNumSlides > 0 &&
                mClient->isConnected() &&
                ((mClient->getCurrentRenderFrame() - mFrameOffset) % mSlideRate) == 0)
            {
                // Increment
                mCurrentSlide = (mCurrentSlide + 1) % mNumSlides;
                updateCurrentSlide();
            }
        }
        
    }
}

void SlideshowApp::mpeFrameUpdate(long serverFrameNumber)
{
    // Your update code.
}

void SlideshowApp::draw()
{
    mClient->draw();
}

void SlideshowApp::mpeFrameRender(bool isNewFrame)
{
    gl::clear(Color(0.0,0.0,0.0));
    
    if (mClient->isAsynchronousClient())
    {
        mParams->draw();
    }
    else
    {
        if (mSlideTexture)
        {
            Vec2i ul = mClient->getVisibleRect().getUpperLeft();
            
            Vec2i texSize = mSlideTexture.getSize();

            // Center vertically
            ul.y += (mClient->getVisibleRect().getHeight() - texSize.y) * 0.5;
            
            Rectf drawRegion(ul, ul + texSize);
            gl::draw(mSlideTexture, drawRegion);
        }
    }
}

void SlideshowApp::mpeMessageReceived(const std::string & message, const int fromClientID)
{
    if (message.find("PAUSE") != -1)
    {
        vector<string> tokens = split(message, "#");
        mIsPaused = stoi(tokens[1]);
    }
    else if (message.find("SLIDERATE") != -1)
    {
        vector<string> tokens = split(message, "#");
        mSlideRate = stoi(tokens[1]);
    }
    else if (message.find("NEXT") != -1)
    {
        mShouldAdvance = true;
    }
    else if (message.find("PREV") != -1)
    {
        mShouldReverse = true;
    }
}

void SlideshowApp::keyDown(KeyEvent event)
{
    //if (mClient->isAsynchronousClient())
    {
        if (event.getCode() == KeyEvent::KEY_LEFT)
        {
            mClient->sendMessage("PREV");
        }
        else if (event.getCode() == KeyEvent::KEY_RIGHT)
        {
            mClient->sendMessage("NEXT");
        }
    }
}

// If you're deploying to iOS, set the Render antialiasing to 0 for a significant
// performance improvement. This value defaults to 4 (AA_MSAA_4) on iOS and 16 (AA_MSAA_16)
// on the Desktop.
#if defined( CINDER_COCOA_TOUCH )
CINDER_APP_NATIVE( SlideshowApp, RendererGl(RendererGl::AA_NONE) )
#else
CINDER_APP_NATIVE( SlideshowApp, RendererGl )
#endif
