#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Camera.h"
#include "cinder/TriMesh.h"
#include "cinder/params/Params.h"
#include "cinder/qtime/QuickTime.h"
#include "Node.h"
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace ci;
using namespace ci::params;
using namespace ci::app;
using namespace std;

const static int kNumFFTChannels = 64;//128;
const static int kMeshDepth = kNumFFTChannels / 2;

class AudioLandscapeApp : public AppNative {
  public:
	void setup();
	void update();
	void draw();
    void loadMovieFile();

    
    CameraPersp         mCamera;
    InterfaceGl         mParams;
    gl::GlslProg        mShaderNodes;
    gl::GlslProg        mShaderSurface;

    vector< vector<Node> >        mNodes;
    float               mNodeRowIndex;
    
    qtime::MovieGlRef   mMovie;
    fs::path            mMoviePath;
    
    float               mCameraZ;
    float               mCameraY;
    float               mCameraFOV;
    float               mAudioScale;
    
    float               mPrevFFTData[kNumFFTChannels];
    Vec3f               mNodeInterval;
    
    boost::posix_time::ptime mTimeUpdated;

};

const static int kWindowWidth = 1200;
const static int kWindowHeight = 400;

void AudioLandscapeApp::setup()
{
    mTimeUpdated = boost::posix_time::microsec_clock::local_time();
    
    mCamera = CameraPersp(kWindowWidth, kWindowHeight, 45.0f, 0, 10000);

    mCameraY = -280;//kWindowHeight * -1;
    mCameraZ = -690;//-730;
    mCameraFOV = 45.0f;
    mAudioScale = 70.0f;//15.0f;
    
    setWindowSize(kWindowWidth, kWindowHeight);
    
    mShaderNodes = gl::GlslProg(loadResource("node_vert.glsl"),
                                loadResource("node_frag.glsl"));
    
    mShaderSurface = gl::GlslProg(loadResource("surface_vert.glsl"),
                                  loadResource("surface_frag.glsl"));
    
    mMoviePath = getResourcePath("gotta_have_it.mp3");
//    mMoviePath = getResourcePath("reich.mp3");

    mParams = InterfaceGl("Grid Settings", Vec2i(200,200));
    mParams.addParam("Camera Y", &mCameraY, "min=-10000.0f max=10000.0f step=10.0f");
    mParams.addParam("Camera Z", &mCameraZ, "min=-10000.0f max=10000.0f step=10.0f");
    mParams.addParam("Camera FOV", &mCameraFOV, "min=0 max=360.0f step=1.0f");
    mParams.addParam("Audio Scale", &mAudioScale, "min=0 max=500.0f step=1.0f");
    mParams.addText("FPS");
    
    mNodeInterval = Vec3f((float)getWindowWidth() / (float)kNumFFTChannels,
                          1.0,
                          (float)getWindowWidth() / (float)kNumFFTChannels);

    for (int i = 0; i < kNumFFTChannels; ++i)
    {
        mPrevFFTData[i] = 0;
    }
    
    for (int y = 0; y < kMeshDepth; ++y)
    {
        vector<Node> yNodes;
        for (int x = 0; x < kNumFFTChannels; ++x)
        {
            Node n;
            Vec3f position = Vec3f(x + 0.5,
                                   0.0,
                                   y + 0.5) * mNodeInterval;
            n.setPosition(position);
            n.setRadius(1.0f);
//            float scalarX = (float)x / (float)kNumFFTChannels;
//            n.setColor(ColorA(1.0f-scalarX, 1.0, scalarX, 1.0f));
            n.setColor(ColorA(1,1,1,1));
            yNodes.push_back(n);
        }
        mNodes.push_back(yNodes);
    }
    
    mNodeRowIndex = 0.0f;

    if(!mMoviePath.empty())
		loadMovieFile();
}

void AudioLandscapeApp::loadMovieFile()
{
	try {
		// load up the movie, set it to loop, and begin playing
		mMovie = qtime::MovieGl::create( mMoviePath );
		mMovie->setLoop();
		//mMovie->play();
        try {
            mMovie->setupMonoFft( kNumFFTChannels );
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
    
    mMovie->play();
}

void AudioLandscapeApp::update()
{
    boost::posix_time::ptime timeNow = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration timeDelta = timeNow - mTimeUpdated;
    mTimeUpdated = timeNow;
    
    float incrementZ = timeDelta.total_milliseconds() / 50.0f;
    mNodeRowIndex += incrementZ;
    
    float edgeRow = ((int)mNodeRowIndex - 1) % kMeshDepth;

    float *fftData = mMovie->getFftData();
    
    for ( int y = 0; y < kMeshDepth; ++y )
    {
        for ( int x = 0; x < kNumFFTChannels; ++x )
        {
            Node & n = mNodes[y][x];
            n.setMaxScale(mAudioScale);
            n.update();
            
            if (y == edgeRow)
            {
                float band = fftData[x];
                float prevBand = mPrevFFTData[x];
                float delta = band - prevBand;
                
                // Give more weight to the lower spectrum
                band *= (1.0f + (((float)x / (float)kNumFFTChannels) * 5.0f));
                
                // Give some extra weight to the delta
                float amplify = ((std::max(0.0f, delta) * 3.0f) + band) / 2.0f;
                n.amplify(amplify);

                mPrevFFTData[x] = band;
            }
            
            // Shift the Z position
            // TODO: This should be handled in a shader
            Vec3f pos = n.getPosition();
            pos.z = fmod(pos.z - (mNodeInterval.z * incrementZ), mNodeInterval.z * kMeshDepth);
            n.setPosition(pos);
            
        }
    }
}

void AudioLandscapeApp::draw()
{
    Vec3f eye(getWindowWidth()/2.f,
              mCameraY,
              mCameraZ);
    
    Vec3f target(getWindowWidth()/2.f,
                 getWindowHeight()/2.f,
                 1000);
    Vec3f up(0, -1, 0);
    mCamera.lookAt(eye, target, up);
    
    mCamera.setPerspective(mCameraFOV, // fov
                           getWindowAspectRatio(), // aspect ratio
                           0.01f, // near
                           10000.0f); // far
    
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    mParams.setOptions("FPS", "label=`FPS: "+to_string((int)getAverageFps())+"`");

    gl::enableAdditiveBlending();
    // gl::enableAlphaBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    // Draw the surface
    // TODO: Make this shader-based
    gl::setMatrices(mCamera);
    
    gl::color(Color(1,1,1));
    
    TriMesh meshSurf;
    
    for ( int y = 0; y < kMeshDepth; ++y )
    {
        for ( int x = 0; x < kNumFFTChannels; ++x )
        {
            Node & node = mNodes[y][x];
            Vec3f position = node.getPosition();

            float scalarX = (float)x / (float)kNumFFTChannels;
            
            float brightness = ci::math<float>::clamp(position.y * -2.5f, 0.0f, 1.0f);
            
            ColorA bandColor(CM_HSV, scalarX, 0.75f, brightness);
            meshSurf.appendColorRgba(bandColor);
            meshSurf.appendVertex(position);
            
            if (x < kNumFFTChannels - 1 && y < kMeshDepth)
            {
                int y2 = (y + 1) % kMeshDepth;
                Node & nextNode = mNodes[y2][x];
                if (nextNode.getPosition().z > position.z)
                {
                    // We don't want them to wrap around
                    int index0 = (y * kNumFFTChannels) + x;
                    int index1 = (y * kNumFFTChannels) + x + 1;
                    int index2 = (y2 * kNumFFTChannels) + x + 1;
                    int index3 = (y2 * kNumFFTChannels) + x;
                    meshSurf.appendTriangle(index0, index1, index2);
                    meshSurf.appendTriangle(index0, index2, index3);
                }
            }
        }
    }
    
    mShaderSurface.bind();
    gl::draw(meshSurf);
    mShaderSurface.unbind();
    
    gl::enableWireframe();
    gl::draw(meshSurf);
    gl::disableWireframe();
    
    /*
    mShaderNodes.bind();
    
    Matrix44f camMat = mCamera.getProjectionMatrix() * mCamera.getModelViewMatrix();
   
    for ( int y = 0; y < kNumFFTChannels; ++y )
    {
        for ( int x = 0; x < kNumFFTChannels; ++x )
        {
            Node & n = mNodes[y][x];
            // Matrix
            Matrix44f matModel = n.getModelMatrix();
            Matrix44f matMVP = camMat * matModel;
            mShaderNodes.uniform("uModelViewProjectionMatrix", matMVP);
            
            // Color
            mShaderNodes.uniform("uColor", n.getColor());
            
            n.draw();
        }
    }
    
    mShaderNodes.unbind();
    */
    

    if (mParams.isVisible())
    {
        mParams.draw();
    }
}

CINDER_APP_NATIVE( AudioLandscapeApp, RendererGl )
