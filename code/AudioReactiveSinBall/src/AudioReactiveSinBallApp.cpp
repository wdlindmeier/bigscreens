#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/MayaCamUI.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/params/Params.h"
#include "Node.h"

const static float InterestingVisualMoments[] =
{
    0.50, 0.65, 0.66, 0.71, 0.96, 0.98, 0.99, 1.00, 1.02, 1.03, 1.04, 1.05, 1.06, 1.07, 1.08, 1.09,
    1.25, 1.26, 1.32, 1.33, 1.43, 1.50, 1.57, 1.65, 1.76, 1.77, 1.82, 1.93, 1.98, 1.99, 2.00, 2.04,
    2.05, 2.06, 2.07, 2.08, 2.09, 2.10, 2.14, 2.26, 2.32, 2.42, 2.43, 2.50, 2.63, 2.64, 2.65, 2.71,
    2.76, 2.89, 2.97, 2.98, 3.01, 3.11, 3.13, 3.14
};

const static float InterestingAudioMoments[] =
{
    14.0,
    31.0,
    34.5,
    51.85,
    72.5,
    79.5,
    112.65,
    121.2,
    130.15,
    162.75,
    170.0,
    190.65,
    197.25,
    214.75,
    218.0,
    236.0
};

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ci::params;

const static int NumFFTChannels = 128;

class AudioReactiveSinBallApp : public AppNative {
  public:
    void prepareSettings( Settings *settings );
	void setup();
    void resize();
    void loadMovieFile();
    void keyDown( KeyEvent event );
    void mouseMove( MouseEvent event );
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
    void mouseUp( MouseEvent event );
    void resetClock();

	void update();
	void draw();
    void drawFFT( const qtime::MovieGlRef & movie, float x, float y, float width, float height );
    
    qtime::MovieGlRef           mMovie;
    MayaCamUI                   mMayaCam;
    
    bool                        mMouseIsDown;
    Vec2f                       mMousePos;
    Vec3f                       mCenter;
    float                       mSphereRadius;
    float                       mSphereRadiusActual;
    float                       mTargetRadius;
    float                       mRadialLerp;
    float                       mRadialDecay;
    float                       mTimestep;
    float                       mStepSize;
    float                       mProgress;
    float                       mNumNodes;
    float                       mAttackWeight;
    float                       mGainWeight;
    bool                        mDidHitInterestingAudio;
    bool                        mDidHitInterestingVisual;
    float                       mMaxBandValue;
    int                         mMaxBand;
    
    InterfaceGl                 mParams;
    
    vector<Node>                mNodes;
    
    fs::path                    mMoviePath;
    
    float                       mProgressStep;
    
    float                       mNextInterestingVisualMoment;
    float                       mPrevInterestingVisualMoment;
    int                         mInterestingVisualMomentIndex;

    float                       mNextInterestingAudioMoment;
    float                       mPrevInterestingAudioMoment;
    int                         mInterestingAudioMomentIndex;
    bool                        mUseInterestingAudioMoments;
    
};

void AudioReactiveSinBallApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 900, 700 );
}


void AudioReactiveSinBallApp::setup()
{
    setFullScreen(true);
    mSphereRadius = 115.0f;
    mSphereRadiusActual = mSphereRadius;
    mTargetRadius = mSphereRadius;
    mTimestep = 0.0;
    mStepSize = 0.01;
    mProgress = 0.0;
    mNumNodes = NumFFTChannels;
    mMouseIsDown = false;
    mMousePos = Vec2f::zero();
    mMaxBandValue = 26.0f;
    mAttackWeight = 0.85f;
    mGainWeight = 0.35f;
    mMaxBand = NumFFTChannels;
    mRadialLerp = 0.65f;
    mRadialDecay = 0.95f;
    mDidHitInterestingAudio = false;
    mDidHitInterestingVisual = false;

    mParams = params::InterfaceGl(getWindow(), "Scene Params", toPixels(Vec2i(250, 250)));
    mParams.addParam("Time Step Size", &mStepSize, "min=0 max=5.0 step=0.01");
    mParams.addParam("Progress", &mProgress, "min=0.0 max=3.2 step=0.01");
    mParams.addParam("Num Nodes", &mNumNodes, "min=0 max=1000");
    mParams.addParam("Sphere Radius", &mSphereRadius, "min=0 max=1000");
    mParams.addParam("Radius Lerp", &mRadialLerp, "min=0 max=1.0 step=0.01");
    mParams.addParam("Radius Decay", &mRadialDecay, "min=0 max=1.0 step=0.01");
    mParams.addParam("Max Node Size", &mMaxBandValue, "min=0 max=200");
    mParams.addParam("Attack Weight", &mAttackWeight, "min=0 max=1.0 step=0.01");
    mParams.addParam("Gain Weight", &mGainWeight, "min=0 max=1.0 step=0.01");
    mParams.addParam("Max Band", &mMaxBand, "min=0 max=" + to_string(NumFFTChannels));
    
    mParams.hide();
    
    mMoviePath = getResourcePath("reich.mp3");

	if( ! mMoviePath.empty() )
		loadMovieFile();
}

void AudioReactiveSinBallApp::resize()
{
    float fov = 45.0f;
    float camDistance = 400.0f; // arbitrary
    int pxWide = getWindowWidth();
    int pxHigh = getWindowHeight();
    mCenter = Vec3f(pxWide * 0.5, pxHigh * 0.5, 0);
    CameraPersp cam(pxWide,
                    pxHigh,
                    fov);
	cam.lookAt(Vec3f( pxWide * 0.5, pxHigh * 0.5, camDistance),
               Vec3f( pxWide * 0.5, pxHigh * 0.5, 0 ) );
	cam.setPerspective(fov,
                       getWindowAspectRatio(),
                       0.1f,
                       5000.0f );
    mMayaCam.setCurrentCam( cam );
}

void AudioReactiveSinBallApp::loadMovieFile()
{    
    mUseInterestingAudioMoments = mMoviePath.string().find("reich.mp3") != -1;
    
	try {
		// load up the movie, set it to loop, and begin playing
		mMovie = qtime::MovieGl::create( mMoviePath );
		mMovie->setLoop();
		//mMovie->play();
        try {
            mMovie->setupMonoFft( NumFFTChannels );
        }
        catch( qtime::QuickTimeExcFft & ) {
            console() << "Unable to setup FFT" << std::endl;
        }
        console() << "FFT Channels: " << mMovie->getNumFftChannels() << std::endl;
	}
	catch( ... ) {
		console() << "Unable to load the movie." << std::endl;
		mMovie->reset();
	}
    
    resetClock();
    
    mMovie->play();
}

void AudioReactiveSinBallApp::resetClock()
{
    float numSeconds = mMovie->getDuration();
    mProgressStep = M_PI / (numSeconds * getFrameRate());
    mProgress = 0.0f;
    mInterestingVisualMomentIndex = 0;
    mNextInterestingVisualMoment = InterestingVisualMoments[mInterestingVisualMomentIndex];
    mPrevInterestingVisualMoment = mNextInterestingVisualMoment * -1;
    
    mInterestingAudioMomentIndex = 0;
    mNextInterestingAudioMoment = InterestingAudioMoments[mInterestingAudioMomentIndex];
    mPrevInterestingAudioMoment = mNextInterestingAudioMoment * -1;
}

void AudioReactiveSinBallApp::update()
{
    if (!mMovie || !mMovie->isPlaying())
    {
        return;
    }
    
    if (mNodes.size() != mNumNodes)
    {
        mNodes.clear();
        for (int i = 0; i < mNumNodes; i++)
        {
            mNodes.push_back(Node((float)i/(float)mNumNodes));
        }
    }
    
    mTimestep += mStepSize;
    mProgress += mProgressStep;

    mDidHitInterestingVisual = false;
    mDidHitInterestingAudio = false;
    
    if (mProgress >= mNextInterestingVisualMoment)
    {
        mDidHitInterestingVisual = true;
        int numVisualMoments = (sizeof(InterestingVisualMoments) / sizeof(float));
        // Move to the next interesting moment
        mPrevInterestingVisualMoment = mNextInterestingVisualMoment;
        mInterestingVisualMomentIndex = (mInterestingVisualMomentIndex + 1) % numVisualMoments;
        mNextInterestingVisualMoment = InterestingVisualMoments[mInterestingVisualMomentIndex];
    }
    
    float movieTime = mMovie->getCurrentTime();
    
    int numAudioMoments = (sizeof(InterestingAudioMoments) / sizeof(float));
    if (movieTime >= mNextInterestingAudioMoment &&
        mInterestingAudioMomentIndex < numAudioMoments)
    {
        mDidHitInterestingAudio = true;
        mPrevInterestingAudioMoment = mNextInterestingAudioMoment;
        mInterestingAudioMomentIndex = (mInterestingAudioMomentIndex + 1) % numAudioMoments;
        mNextInterestingAudioMoment = InterestingAudioMoments[mInterestingAudioMomentIndex];
    }

    for (int i = 0; i < mNumNodes; i++)
    {
        float scalarNode = (float)i/mNumNodes;
        int fftChannel = mMaxBand * scalarNode;
        float *fftData = mMovie->getFftData();
        float bandValue = fftData[fftChannel];
        
        const static float kMinBandValue = 2.0f;
        Node & node = mNodes[i];
        node.update(bandValue,
                    kMinBandValue,
                    mMaxBandValue,
                    mAttackWeight,
                    mGainWeight);
    }
}

void AudioReactiveSinBallApp::draw()
{
    bool isMoviePlaying = mMovie && mMovie->isPlaying();
    
    float scalarVisualMoment = 0.0f;
    float scalarAudioMoment = 1.0f;
    float midVisualMoment = 0.0f;
    
    if (isMoviePlaying)
    {
        scalarVisualMoment = (mProgress - mPrevInterestingVisualMoment) /
                             (mNextInterestingVisualMoment - mPrevInterestingVisualMoment);
        
        if (scalarVisualMoment != 0.0)
        {
            midVisualMoment = fabs(0.5f - scalarVisualMoment) / 0.5f;
        }        
        
        if (mUseInterestingAudioMoments)
        {
            float secsSincePrevMoment = mMovie->getCurrentTime() - mPrevInterestingAudioMoment;
            if (mMovie->getCurrentTime() < mNextInterestingAudioMoment)
            {
                scalarAudioMoment = secsSincePrevMoment /
                (mNextInterestingAudioMoment - mPrevInterestingAudioMoment);
            }
            

            if (mDidHitInterestingAudio)
            {
                // Expand the target
                mTargetRadius = mSphereRadius + (mSphereRadius * 0.2);
            }
            else
            {
                // Shrink the target
                mTargetRadius = ci::lerp(mSphereRadius, mTargetRadius, mRadialDecay);
            }
            
            mSphereRadiusActual = ci::lerp(mTargetRadius, mSphereRadiusActual, mRadialLerp);
        }
        else
        {
            scalarAudioMoment = 1.0f;
        }
    }
    
    // Flatten out to draw fft
    gl::setMatricesWindow(getWindowWidth(), getWindowHeight());
    
    // Draw sound stuff
	gl::clear( Color::gray( 0.35f ) );
    gl::enableAlphaBlending();
    
    Vec2f initPos( 15, 100 );
    Vec2f pos = initPos;

    if (isMoviePlaying)
    {
        float drawHeight = 100.0f;
        drawFFT(mMovie, 0,
                getWindowHeight() - drawHeight,
                getWindowWidth(), drawHeight);
    }

    // 3D mode
    gl::setMatrices( mMayaCam.getCamera() );
    
    // Draw Balls    
    gl::enableDepthWrite();
    gl::enableDepthRead();    

    float nodeSize = mNodes.size();
    for(int i=0;i<nodeSize;i++)
    {
        float offset = mTimestep + i;

        float yProgress = cos(offset * mProgress);
        float zProgress = sin(offset * mProgress);
        
        float y = mCenter.y + (yProgress * mSphereRadiusActual);
        float x = mCenter.x + (sin(offset) * mSphereRadiusActual * zProgress);
        float z = mCenter.z + (cos(offset) * mSphereRadiusActual * zProgress);
        
        /*
         // Old world.
         // Colors morph.
         // Looks nice.
        gl::color(1.0-zProgress,
                  yProgress,
                  zProgress);
        */
        
        Node & n = mNodes[i];
        Vec3f position(x,y,z);
        Vec3f progress(fmod(offset * mProgress, (float)nodeSize) / nodeSize,
                       yProgress,
                       zProgress);
        n.draw(position, progress);        
    }
    
    gl::disableDepthWrite();

    // The sphere is clear when it's visually interesting
    // The sphere is white when audibly interesting
    float whiteness = 1.0-scalarAudioMoment;
    gl::color(ColorA(whiteness,
                     whiteness,
                     whiteness,
                     1.0-midVisualMoment));
    gl::drawSphere(mCenter, mSphereRadiusActual, 50);
    
    gl::disableDepthRead();
    
    // Draw the interface
    if (mParams.isVisible())
    {
        mParams.draw();
    }
}

void AudioReactiveSinBallApp::drawFFT( const qtime::MovieGlRef & movie, float x, float y, float width, float height )
{
	if( ! movie->getNumFftChannels() )
		return;
	
	float bandWidth = width / movie->getNumFftBands();
	float *fftData = movie->getFftData();
    gl::color( Color( 0.225, 0.225, 0.225 ) );
	for( uint32_t band = 0; band < movie->getNumFftBands(); ++band ) {
		float bandHeight = height / 3.0f * fftData[band];
		gl::drawSolidRect( ci::Rectf(x + band * bandWidth,
                                     y + height - bandHeight,
                                     x + band * bandWidth + bandWidth,
                                     y + height ) );
	}
}

void AudioReactiveSinBallApp::keyDown( KeyEvent event )
{
    char c = event.getChar();
    
    switch ( c )
    {
        case ' ':
        {
            loadMovieFile();
        }
        break;
        case 'o':
        {
            mMoviePath = getOpenFilePath();
            loadMovieFile();
        }
            break;
        case 'f':
        case KeyEvent::KEY_ESCAPE:
            setFullScreen(!isFullScreen());
            break;
    }
}

void AudioReactiveSinBallApp::mouseMove( MouseEvent event )
{
}

void AudioReactiveSinBallApp::mouseDown( MouseEvent event )
{
    mParams.show();
}

void AudioReactiveSinBallApp::mouseDrag( MouseEvent event )
{
}

void AudioReactiveSinBallApp::mouseUp( MouseEvent event )
{
    mMouseIsDown = false;
}

CINDER_APP_NATIVE( AudioReactiveSinBallApp, RendererGl )
