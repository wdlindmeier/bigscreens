#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "MPEApp.hpp"
#include "MPEClient.h"
#include "cinder/params/Params.h"
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
    
	void update();
    void mpeFrameUpdate(long serverFrameNumber);
	
    void draw();
    void mpeFrameRender(bool isNewFrame);
    
    void keyDown(KeyEvent event);
    
    MPEClientRef mClient;
    
    void loadAllSlides();
    
    fs::path mSlidesDirectory;

    int mSlideRate;
    int mCurrentSlide;
    int mNumSlides;
    bool mIsPaused;
    
    params::InterfaceGlRef mParams;
    
    vector<fs::path>  mSlidePaths;
    
    gl::Texture mSlideTexture;
    
};

void SlideshowApp::setup()
{
    mClient = MPEClient::Create(this);
    mClient->setIsRendering3D(false);
    
    mSlideRate = 60;
    mNumSlides = 0;
    mCurrentSlide = 0;
    mIsPaused = false;
    
    mParams = params::InterfaceGl::create("Slideshow", Vec2f(200, 50));
    mParams->addParam("Rate (frames)", &mSlideRate, "min=1");
    mParams->addParam("Pause", &mIsPaused);
    
    mSlidesDirectory = getAppPath() / ".." / "Slides";
    
    if ( !fs::exists( mSlidesDirectory ) )
    {
        fs::create_directories( mSlidesDirectory );
    }
    
    loadAllSlides();
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
}

void SlideshowApp::update()
{
    if (!mClient->isConnected() && (getElapsedFrames() % 60) == 0)
    {
        mClient->start();
    }
    // NOTE: 
    if (!mIsPaused &&
        mNumSlides > 0 &&
        mClient->isConnected() &&
        mClient->getCurrentRenderFrame() % mSlideRate == 0)
    {
        // Increment
        mCurrentSlide = (mCurrentSlide + 1) % mNumSlides;
        fs::path slidePath = mSlidePaths[mCurrentSlide];
        mSlideTexture = loadImage( loadFile( slidePath ) );
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
    gl::clear(Color(0.5,0.5,0.5));
    // Your render code.
    if (mSlideTexture)
    {
        gl::draw(mSlideTexture);
    }
    
    if (mParams->isVisible())
    {
        mParams->draw();
    }
}

void SlideshowApp::keyDown(KeyEvent event)
{
    if (event.getCode() == KeyEvent::KEY_LEFT)
    {
        if (mNumSlides > 0)
        {
            mCurrentSlide = (mCurrentSlide - 1) % mNumSlides;
        }
    }
    else if (event.getCode() == KeyEvent::KEY_RIGHT)
    {
        if (mNumSlides > 0)
        {
            mCurrentSlide = (mCurrentSlide + 1) % mNumSlides;
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
