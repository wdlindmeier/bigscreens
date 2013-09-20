#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include <boost/iterator/filter_iterator.hpp>
#include "CinderOpenCv.h"
#include <algorithm>
#include <string>

using namespace ci;
using namespace ci::app;
using namespace std;

class ImageResizerApp : public AppNative
{
  public:
	void setup();
	void draw();
    void update();
    void loadAllImages();
	
	gl::Texture	mTexture;
    cv::Mat mInputMat;
    fs::path mInputPath;
    fs::path mOutputPath;
    
    std::vector<fs::path> mInputFiles;
    fs::path mResizeFile;
};

void ImageResizerApp::setup()
{
    // These paths are specific to Bill and Ryans repo
    mInputPath = string(SRCROOT) + "/../../../moodboard";
    mOutputPath = string(SRCROOT) + "/../../../moodboard_scaled";

    // NOTE: If you want to run this app on other image sets,
    // you can un-comment the following code which will look for
    // images in a folder called "Original" and write them into
    // a folder called "Resized". All in the same folder as the app itself.
    
    /*
    mInputPath = getAppPath() / ".." / "Original";
    mOutputPath = getAppPath() / ".." / "Resized";
    
    if ( !fs::exists( mInputPath ) )
    {
        fs::create_directories( mInputPath );
    }

    if ( !fs::exists( mOutputPath ) )
    {
        fs::create_directories( mOutputPath );
    }
    */
    
    loadAllImages();
}

void ImageResizerApp::loadAllImages()
{
    if ( fs::exists(mInputPath) && fs::is_directory(mInputPath) )
    {
        console() << "Found the input path: " << mInputPath << std::endl;
    }
    else
    {
        console() << "ERROR: Couldn't find the input directory: " << mInputPath << std::endl;
        return;
    }
    if ( fs::exists(mOutputPath) && fs::is_directory(mOutputPath) )
    {
        console() << "Found the output path: " << mOutputPath << std::endl;
    }
    else
    {
        console() << "ERROR: Couldn't find the output directory: " << mOutputPath << std::endl;
        return;
    }

    fs::directory_iterator dir_first(mInputPath), dir_last;
    
    // Filter images
    auto pred = [](const fs::directory_entry& p)
    {
        string filename = string(p.path().filename().c_str());
        std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
        return fs::is_regular_file(p) &&
               (filename.find( ".png" ) != -1 ||
                filename.find( ".gif" ) != -1 ||
                filename.find( ".jpeg" ) != -1 ||
                filename.find( ".jpg" ) != -1);
    };
    
    std::copy(make_filter_iterator(pred, dir_first, dir_last),
              make_filter_iterator(pred, dir_last, dir_last),
              std::back_inserter(mInputFiles));
}

const static int kOutputWidth = 11520;
const static int kOutputHeight = 947;

void ImageResizerApp::update()
{
    // FIRST resize the previous input file
    if (mResizeFile != fs::path())
    {
        console() << "Resizing " << mResizeFile << std::endl;
        
        // Figure out the scale
        cv::Size inputSize = mInputMat.size();
        
        cv::Size scaledSize = inputSize;
        
        // Only scale the image if it's shorter than the output height
        bool scaleUp = inputSize.height < kOutputHeight;
        if (scaleUp)
        {
            float heightRatio = (float)kOutputHeight / (float)inputSize.height;
            scaledSize.width = mInputMat.cols * heightRatio;
            scaledSize.height = kOutputHeight;
        }
        
        cv::Mat scaledInput(scaledSize, mInputMat.type());
        cv::Mat flippedScaledInput(scaledSize, mInputMat.type());
        if (scaleUp)
        {
            cv::resize(mInputMat, scaledInput, scaledSize, 1, 1, cv::INTER_AREA);
        }
        else
        {
            scaledInput = mInputMat;
        }
        
        cv::flip(scaledInput, flippedScaledInput, 1);
        
        // Create the output container
        cv::Mat outMat(cv::Size(kOutputWidth, kOutputHeight), mInputMat.type());
        
        int newX = 0;
        bool flip = false;
        while (newX < kOutputWidth)
        {
            // Get the region of interest
            int regionWidth = std::min<int>(scaledSize.width, kOutputWidth - newX);
            cv::Mat region = outMat(cv::Rect(newX, 0,
                                             regionWidth,
                                             kOutputHeight));
            newX = newX + scaledSize.width;
            
            cv::Rect copyRegion(0,
                                (int)((scaledSize.height - kOutputHeight) * 0.5),
                                regionWidth,
                                kOutputHeight);
            if (flip)
            {
                flippedScaledInput(copyRegion).copyTo(region);
            }
            else
            {
                scaledInput(copyRegion).copyTo(region);
            }
            //flip = !flip;
        }

        // Save to output
        fs::path writePath = mOutputPath / mResizeFile.filename().string();
        writeImage(writePath, fromOcv(outMat));
        
        console() << "Complete." << std::endl;
    }
    
    // THEN pick the next one
    if (mInputFiles.begin() != mInputFiles.end())
    {
        mResizeFile = *mInputFiles.begin();
        mInputFiles.erase(mInputFiles.begin());

        // Show the texture on the screen when we resize it.
        try
        {
            Surface input = loadImage( loadFile( mResizeFile ) );
            mInputMat = toOcv( input );
            mTexture = gl::Texture( input );
        }
        catch(ImageIoExceptionFailedLoad e)
        {
            console() << "ERROR: Couldn't load file. Ignoring. " << mResizeFile << std::endl;
            // Clear out the path
            mResizeFile = fs::path();
        }
    }
    else
    {
        mResizeFile = fs::path();
        console() << "DONE!" << std::endl;
        exit(0);
    }
}

void ImageResizerApp::draw()
{
	gl::clear();
    if (mTexture)
    {
        Rectf destRect = Rectf(0,0,getWindowWidth(),getWindowHeight());
        Rectf inputRect = Rectf(mTexture.getBounds()).getCenteredFit( destRect, true);
        gl::draw( mTexture,  inputRect);
    }
}

CINDER_APP_NATIVE( ImageResizerApp, RendererGl )
