#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "cinder/qtime/QuickTimeGl.h"
#include <fstream>

using namespace ci;
using namespace ci::app;
using namespace std;

const static int kNumFFTBands = 64;
const static int kRecordNumFrames = 12600;

class FFTGeneratorApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    qtime::MovieGlRef    mSoundtrack;
    float ffts[kRecordNumFrames][kNumFFTBands];
};

void FFTGeneratorApp::setup()
{
    setFrameRate(60.0f);
    setWindowSize(10, 10);
    fs::path audioPath = getResourcePath("intriguing_possibilities_clipped_fade.aiff");
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

    try
    {
        mSoundtrack->setupMonoFft( kNumFFTBands );
    }
    catch( qtime::QuickTimeExcFft & )
    {
        console() << "Unable to setup FFT" << std::endl;
    }
    console() << "FFT Channels: " << mSoundtrack->getNumFftChannels() << std::endl;
    mSoundtrack->seekToStart();
    mSoundtrack->play();
}

void FFTGeneratorApp::mouseDown( MouseEvent event )
{
}

void FFTGeneratorApp::update()
{
    // write this shit out
    int frameNum = getElapsedFrames();
    if (frameNum >= kRecordNumFrames)
    {
        console() << "WRITING FILE\n";
        // Wrap it up
        std::ofstream oStream( (getHomeDirectory() / "Documents" / "fftData.h").string() );
        oStream << "const static float kCannedFFTData[" << kRecordNumFrames << "][" << kNumFFTBands << "] = {\n";
        for (int i = 0; i < kRecordNumFrames; ++i)
        {
            oStream << "{";
            for (int j = 0; j < kNumFFTBands; ++j)
            {
                oStream << ffts[i][j] << ",";
            }
            oStream << "},\n";
        }
        oStream << "};";
        oStream.close();
        exit(0);
    }
    else
    {
        float *fftData = mSoundtrack->getFftData();
        for (int i = 0; i < kNumFFTBands; ++i)
        {
            ffts[frameNum][i] = fftData[i];
        }
    }
}

void FFTGeneratorApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    // console() << getAverageFps() << "\n";
}

CINDER_APP_NATIVE( FFTGeneratorApp, RendererGl )
