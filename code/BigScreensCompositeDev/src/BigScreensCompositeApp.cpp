#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
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
#include "TankBlinkingContent.h"
#include "FinalBillboard.h"
#include "ConvergenceContent.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace mpe;
using namespace bigscreens;

static const std::string kContentKeyTankSpin = "tankSpin";
static const std::string kContentKeyTankOverhead = "tankOverhead";
static const std::string kContentKeyTankHeightmap = "tankHeightmap";
static const std::string kContentKeyTankWide = "tankWide";
static const std::string kContentKeyTankHorizon = "tankHorizon";
static const std::string kContentKeyTanksConverge = "tanksConverge";
static const std::string kContentKeyPerlin = "perlin";

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
	void updateContentForRender(const TimelineContentInfo & contentInfo);
    
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
    RenderableContentRef mTankContent;
    RenderableContentRef mTankContentHeightmap;
    RenderableContentRef mPerlinContent;
//    RenderableContentRef mTankContentConverge;
    RenderableContentRef mTextureContentBlank;
    RenderableContentRef mConvergenceContent;
    
    OutLineBorderRef     mOutLine;
    
    map<int, TimelineContentInfo>  mCurrentContentInfo;
	
	FinalBillboardRef    mFinalBillboard;
	gl::FboRef           mFbo;
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
    
    // console() << "IS_IAC ? " << IS_IAC << "\n";
    
    GridLayoutTimeline *timeline = new GridLayoutTimeline(SharedGridAssetPath(!IS_IAC),
                                                          kScreenScale);
    mTimeline = GridLayoutTimelineRef(timeline);
    
    mOutLine = OutLineBorderRef(new OutLineBorder());
	
	mFinalBillboard = FinalBillboardRef( new FinalBillboard() );
	
	gl::Fbo::Format mFboFormat;
	mFboFormat.colorTexture().depthBuffer().samples( 16 );
	mFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight(), mFboFormat );
    
	mFbo->bindFramebuffer();
	gl::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	mFbo->unbindFramebuffer();
	
	cout << mFbo->getSize() << " " << getWindowSize();
	
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
    mCurrentContentInfo.clear();
    mTimeline->reload();
    mTimeline->restart();
    if (!mTimeline->isPlaying())
    {
        mSoundtrack->stop();
    }
    mSoundtrack->seekToStart();
    
    static_pointer_cast<TankContent>(mTankContent)->reset();
    static_pointer_cast<TankHeightmapContent>(mTankContentHeightmap)->reset();
    static_pointer_cast<PerlinContent>(mPerlinContent)->reset();

    // NOTE: This assumes the last layout is convergence and the second to last
    // is the grid that is merged
    std::vector<GridLayout> & layouts = mTimeline->getGridLayouts();
    GridLayout & penultimateLayout = layouts[layouts.size() - 2];
    static_pointer_cast<ConvergenceContent>(mConvergenceContent)->reset(penultimateLayout);
}

void BigScreensCompositeApp::loadAssets()
{
    TankContent *tank = new TankContent();
    tank->load("T72.obj");
    mTankContent = RenderableContentRef(tank);
    
    TankHeightmapContent *tankHeightmap = new TankHeightmapContent();
    tankHeightmap->load("T72.obj");
    mTankContentHeightmap = RenderableContentRef(tankHeightmap);

    ConvergenceContent *converge = new ConvergenceContent();
    converge->load(TRANSITION_FADE);
    Vec2i masterSize = mClient->getMasterSize();
    converge->setContentRect(Rectf(0,0,masterSize.x, masterSize.y));
    mConvergenceContent = RenderableContentRef(converge);
    
    PerlinContent *perlinContent = new PerlinContent();
    mPerlinContent = RenderableContentRef(perlinContent);

    TextureContent *texBlank = new TextureContent();
    texBlank->load("blank_texture.png");
    mTextureContentBlank = RenderableContentRef(texBlank);
    
}

void BigScreensCompositeApp::loadAudio()
{
    fs::path audioPath = SharedAssetPath(!IS_IAC) / "audio" / "intriguing_possibilities_clipped_fade.aiff";
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
    if (contentName == kContentKeyTankSpin ||
        contentName == kContentKeyTankOverhead ||
        contentName == kContentKeyTankWide ||
        contentName == kContentKeyTankHorizon)
    {
        return mTankContent;
    }
    else if( contentName == kContentKeyTankHeightmap)
    {
        return mTankContentHeightmap;
    }
    else if (contentName == kContentKeyTanksConverge)
    {
        return mConvergenceContent;
    }
    else if (contentName == kContentKeyPerlin)
    {
        return mPerlinContent;
    }

    // Default is a blank texture
    return mTextureContentBlank;
}

#pragma mark - Input events

void BigScreensCompositeApp::keyUp(KeyEvent event)
{
    if (mClient->isConnected())
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
}

void BigScreensCompositeApp::updateContentForRender(const TimelineContentInfo & contentInfo)
{
    long long contentElapsedFrames = contentInfo.numRenderFrames;
    Vec2f tankGroundOffset(0, contentElapsedFrames * -0.05f);
    RenderableContentRef content = contentInfo.contentRef;
    content->setFramesRendered(contentElapsedFrames);
    
    if (contentInfo.contentKey == kContentKeyTankSpin)
    {
        // Blinking tank rotation
        float tankRotation = contentElapsedFrames * 0.01;
        shared_ptr<TankContent> tank = static_pointer_cast<TankContent>(content);
        tank->setGroundOffset(Vec2f::zero()); // NOTE: Keeping ground still
        tank->setGroundIsVisible(true);
        tank->resetPositions();
        tank->update([=](CameraPersp & cam){
            // cam.setPerspective( 45.0f, getWindowAspectRatio(), .01, 40000 );
            float camX = cosf(tankRotation) * 1000;
            float camZ = sinf(tankRotation) * 1000;
            cam.lookAt( Vec3f( camX, 400, camZ ), Vec3f( 0, 100, 0 ) );
        });
    }
    else if (contentInfo.contentKey == kContentKeyTankOverhead)
    {
        // Flat texture ground
        float tankDistance = sinf(contentElapsedFrames * 0.0025);
        shared_ptr<TankContent> tank = static_pointer_cast<TankContent>(content);
        tank->setGroundOffset(tankGroundOffset);
        tank->setGroundIsVisible(true);
        tank->resetPositions();
        tank->update([=](CameraPersp & cam){
            // cam.setPerspective( 45.0f, getWindowAspectRatio(), .01, 40000 );
            // Zoom in and out
            float camZ = tankDistance * 500;
            cam.lookAt(Vec3f( 100, 500, camZ ),
                       Vec3f( 0, 100, 0 ) );
        });
    }
    else if (contentInfo.contentKey == kContentKeyTankWide)
    {
        // Tank content wide shot
        shared_ptr<TankContent> tank = static_pointer_cast<TankContent>(content);
        tank->setGroundOffset(tankGroundOffset);
        tank->setGroundIsVisible(true);
        tank->resetPositions();
        tank->update([=](CameraPersp & cam){
            // cam.setPerspective( 45.0f, getWindowAspectRatio(), .01, 40000 );
            float camX, camY, camZ;
            switch (CLIENT_ID)
            {
                case 0:
                    camY = 100;
                    camZ = (contentElapsedFrames * 2.9) - 3000;
                    camX = -1000 + camZ;
                    break;
                case 1:
                    camY = 100;
                    camZ = 3000 - (contentElapsedFrames * 4);
                    camX = 500;// + camZ;
                    break;
                case 2:
                    camY = 100;
                    camZ = 3000 - (contentElapsedFrames * 2.9);
                    camX = -1000 + -camZ;
                    break;
            }
            
            cam.lookAt(Vec3f(camX, camY, camZ),
                      Vec3f( 0, 100, 0 ) );
        });
    }
    else if (contentInfo.contentKey == kContentKeyTankHorizon)
    {
        // Tank content horizon shot
        shared_ptr<TankContent> tank = static_pointer_cast<TankContent>(content);
        tank->setTankPosition(Vec3f(0, 0, -45000 * (1.0-(contentElapsedFrames/4000.0))));
        tank->setGroundIsVisible(false);
        tank->update([=](CameraPersp & cam)
        {
            // Nearly flat
            cam.setPerspective(5, getWindowAspectRatio(), 0.01, 150000);
            float camX = -80000;
            float camY = 100;
            float camZ = 0;
            cam.lookAt(Vec3f( camX, camY, camZ ),
                       Vec3f( 0, camY, camZ ) );
        });
    }
    else if (contentInfo.contentKey == kContentKeyTanksConverge)
    {
        // Nothing to see here
    }
    else if (contentInfo.contentKey == kContentKeyPerlin)
    {
        // Perlin
        // NOTE: The movement here doesn't map to PX.
        // The image /should/ shift 1px per frame...
        shared_ptr<PerlinContent> perlin = static_pointer_cast<PerlinContent>(content);
        perlin->update(Vec2f(0.0, -0.2));
    }
    else if (contentInfo.contentKey == kContentKeyTankHeightmap)
    {
        // Heightmap
        shared_ptr<TankHeightmapContent> tank = static_pointer_cast<TankHeightmapContent>(content);
        // tank->getCamera().setPerspective( 45.0f, getWindowAspectRatio(), .01, 40000 );

        Vec3f tankPosition = tank->getTankPosition() + Vec3f(0, 0, 20);
        tank->setTankPosition(tankPosition);
        tank->update([=](CameraPersp & cam)
        {
            cam.lookAt(Vec3f( 0, 600, -1000 ) + tankPosition,
                       Vec3f( 0, 100, 0 ) + tankPosition);
        });

    }
}

#pragma mark - Render

void BigScreensCompositeApp::draw()
{
    mClient->draw();
}

void BigScreensCompositeApp::mpeFrameRender(bool isNewFrame)
{
    gl::clear( Color( 0, 0, 0 ), true );
    
    // Create a new map of render times.
    // The old one will be replaced.
    map<int, TimelineContentInfo>  newContentInfo;
    
    // NOTE: DON'T transition on the last layout (this is the convergence scene)
    int layoutSize = mTimeline->getGridLayouts().size();
    const int convergenceLayoutIndex = layoutSize - 1;
    bool shouldTransition = mTimeline->getCurrentFrame() != convergenceLayoutIndex;
    
    std::map<int, TimelineContentInfo > renderContent = mTimeline->getRenderContent(this,
                                                                                    shouldTransition);
    
    Vec2i windowSize = mClient->getVisibleRect().getSize();
    Vec2i offset = mClient->getVisibleRect().getUpperLeft();

	// mFbo->bindFramebuffer();
    
    for (auto & kv : renderContent)
    {
        int contentID = kv.first;
        TimelineContentInfo renderMe = kv.second;
        
        // Update the content info even when it's not being drawn
        if (mClient->isConnected() &&
            mTimeline->isPlaying())
        {
            if (mCurrentContentInfo.count(contentID))
            {
                TimelineContentInfo prevContentInfo = mCurrentContentInfo[contentID];
                renderMe.numRenderFrames = prevContentInfo.numRenderFrames + 1;
            }
            else
            {
                renderMe.numRenderFrames = 1;
            }
            newContentInfo[contentID] = renderMe;
        }
        
        // Check if this should be rendered at all.
        if (rectsOverlap(renderMe.rect, mClient->getVisibleRect()))
        {
            // NOTE: Do the updates >>> here <<<
            // Some module won't be rendered (and therefor shouldn't be updated)
            // and other modules will be rendered (and updated) more than once.
            
            updateContentForRender(renderMe);

            SceneWindow scene(renderMe.contentRef,
                              renderMe.rect,
                              windowSize);
            
            scene.render(offset);

            // NOTE: Convergence takes care of it's own frames
            if (renderMe.contentKey != kContentKeyTanksConverge)
            {
                mOutLine->render();
            }
        }
    }
    
    // mFbo->unbindFramebuffer();
	
	// mFinalBillboard->draw( mFbo->getTexture() );
	
    if (mIsDrawingColumns)
    {
        renderColumns();
    }
    
    mCurrentContentInfo = newContentInfo;
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
