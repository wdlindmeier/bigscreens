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
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Shader.h"
#include "OutLineBorder.hpp"
#include "PerlinContent.h"
#include "cinder/Rand.h"
#include "cinder/qtime/QuickTimeGl.h"
#include "TankBlinkingContent.h"
#include "TankConvergenceContent.h"
#include "FinalBillboard.h"
#include "ConvergenceContent.h"
#include "ContentProvider.h"
#include "DumbTankContent.h"
#include "TankMultiOverheadContent.h"
#include "OpponentContent.h"
#include "TextLoopContent.h"
#include "StaticContent.h"
#include "LandscapeContent.h"
#include "cinder/ImageIo.h"
#include "cinder/ip/Flip.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace mpe;
using namespace bigscreens;

class BigScreensCompositeApp : public AppNative, public MPEApp, public SceneContentProvider
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
    
    // Clock
    void startConvergenceClock();
    
    // Update
	void update();
    void mpeFrameUpdate(long serverFrameNumber);
    void updatePlaybackState();
	void updateContentForRender(const TimelineContentInfo & contentInfo, const int contentID);
    
    // Draw
    void draw();
    void mpeFrameRender(bool isNewFrame);
    void renderColumns();
    
    // Messages
    void mpeMessageReceived(const std::string &message, const int fromClientID);
    void newLayoutWasSet();
    
    // Content Provider
    RenderableContentRef contentForKey(const std::string & contentName);
    float * getFFTData();
    float getFFTDataForChannel(const int channel);

    // Vars
    MPEClientRef mClient;
    
    GridLayoutTimelineRef mTimeline;
    
    bool mIsDrawingColumns;
    int mLayoutIndex;
    
    // Audio
    qtime::MovieGlRef    mSoundtrack;
    
    // Content
    RenderableContentRef mTankContent;
    RenderableContentRef mPerlinContent;
    RenderableContentRef mSingleTankConvergeContent;
    RenderableContentRef mTextureContentBlank;
    RenderableContentRef mConvergenceContent;
    RenderableContentRef mDumbTankContent;
    RenderableContentRef mMultiOverheadContent;
    RenderableContentRef mOpponentContent;
    RenderableContentRef mTextLoopContent;
    RenderableContentRef mStaticContent;
    RenderableContentRef mLandscapeContent;
    
    OutLineBorderRef     mOutLine;
    
    map<int, TimelineContentInfo>  mCurrentContentInfo;
	
	FinalBillboardRef    mFinalBillboard;
	gl::FboRef           mFbo;
    
    int                  mConvergenceLayoutIndex;
    int                  mPreConvergenceLayoutIndex;
    long long            mMSElapsedConvergence;
    long long            mMSConvergenceBegan;
    
    bool                 mShouldFire;
    float                mScalarTimelineProgress;
};

#pragma mark - Setup

void BigScreensCompositeApp::prepareSettings(Settings *settings)
{
#if IS_IAC
    settings->setFullScreen();
#endif
    SceneContentProvider::sharedContentProvider(this);
}

void BigScreensCompositeApp::setup()
{
    mClient = MPEClient::Create(this);
    mClient->setIsRendering3D(false);
    mClient->setIsScissorEnabled(false);
    
    // console() << "IS_IAC ? " << IS_IAC << "\n";
    
    GridLayoutTimeline *timeline = new GridLayoutTimeline(SharedGridAssetPath(!IS_IAC),
                                                          kScreenScale);
    
    gl::Fbo::Format mFormat;
    mFormat.colorTexture().depthBuffer( GL_DEPTH_COMPONENT32F ).samples(4);
    mFbo = gl::Fbo::create(mClient->getVisibleRect().getWidth(),
                           mClient->getVisibleRect().getHeight(), mFormat );
    
    mFbo->bindFramebuffer();
    gl::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    mFbo->unbindFramebuffer();
    
    mTimeline = GridLayoutTimelineRef(timeline);
    
    mOutLine = OutLineBorderRef(new OutLineBorder());
	
	mFinalBillboard = ActorContentProvider::getFinalBillboard();
	
    mShouldFire = false;
	
    mIsDrawingColumns = false;

    loadAudio();
    loadAssets();
    reload();
}

void BigScreensCompositeApp::shutdown()
{    
}

#pragma mark - Load

extern long MSConvergeBeforeCameraMerge;
extern long MSCamerasConverge;
extern float MaxExplosionScale = kDefaultExplosionScale;

void BigScreensCompositeApp::reload()
{
    mScalarTimelineProgress = 0;
    mLayoutIndex = -1;
    mCurrentContentInfo.clear();
    mTimeline->reload();
    mTimeline->restart();
    
    if (!mTimeline->isPlaying())
    {
        mSoundtrack->stop();
    }
    mSoundtrack->seekToStart();
    
    static_pointer_cast<TankContent>(mTankContent)->reset();
    static_pointer_cast<PerlinContent>(mPerlinContent)->reset();
    static_pointer_cast<TankConvergenceContent>(mSingleTankConvergeContent)->reset();
    static_pointer_cast<DumbTankContent>(mDumbTankContent)->reset();
    static_pointer_cast<TankMultiOverheadContent>(mMultiOverheadContent)->reset();
    // NOTE: This assumes the last layout is convergence and the second to last
    // is the grid that is merged
    
    vector<GridLayout> & layouts = mTimeline->getGridLayouts();
    int layoutSize = layouts.size();
    mConvergenceLayoutIndex = layoutSize - 1;
    mPreConvergenceLayoutIndex = mConvergenceLayoutIndex - 1;
    
    mMSElapsedConvergence = 0;
    mMSConvergenceBegan = 0;

    GridLayout & convergenceLayout = layouts[mConvergenceLayoutIndex];
    GridLayout & penultimateLayout = layouts[mPreConvergenceLayoutIndex];

    long long convergeTimestamp = convergenceLayout.getTimestamp();
    long long preConvergeTimestamp = penultimateLayout.getTimestamp();
    long long durationPreConverge = convergeTimestamp - preConvergeTimestamp;
    
    MSConvergeBeforeCameraMerge = durationPreConverge;
    MSCamerasConverge = kMSFullConvergence - MSConvergeBeforeCameraMerge;
    
    console() << "kMSFullConvergence: " << kMSFullConvergence <<
                 " MSConvergeBeforeCameraMerge: " << MSConvergeBeforeCameraMerge <<
                 " MSCamerasConverge: " << MSCamerasConverge << endl;
    
    static_pointer_cast<ConvergenceContent>(mConvergenceContent)->reset(penultimateLayout);
    
    // Now that the timeline is loaded, pre-load the text modules
    // TODO: Make this it's own method
    shared_ptr<TextLoopContent> textContent = static_pointer_cast<TextLoopContent>(mTextLoopContent);
    for (GridLayout & gl : layouts)
    {
        for (ScreenRegion & sr : gl.getRegions())
        {
            if (rectsOverlap(mClient->getVisibleRect(), sr.rect))
            {
                if (sr.contentKey.compare(0, kContentKeyTextPrefix.length(), kContentKeyTextPrefix) == 0)
                {
                    TextContentProvider::TextTimelineAndHeight timeAndHeight =
                    TextContentProvider::textTimelineForContentKey(sr.contentKey);
                    textContent->setTextForContentID(timeAndHeight.timeline,
                                                     sr.timelineID,
                                                     timeAndHeight.absoluteLineHeight * kScreenScale);
                }
            }
        }
    }

}

void BigScreensCompositeApp::loadAssets()
{
    TankContent *tank = new TankContent();
    tank->load();
    mTankContent = RenderableContentRef(tank);
    
    ConvergenceContent *converge = new ConvergenceContent();
    converge->load();
    Vec2i masterSize = mClient->getMasterSize();
    converge->setContentRect(Rectf(0,0,masterSize.x, masterSize.y));
    mConvergenceContent = RenderableContentRef(converge);
    
    PerlinContent *perlinContent = new PerlinContent();
    mPerlinContent = RenderableContentRef(perlinContent);

    TextureContent *texBlank = new TextureContent();
    texBlank->load("blank_texture.png");
    mTextureContentBlank = RenderableContentRef(texBlank);
    
    TankConvergenceContent *tankConverge = new TankConvergenceContent();
    tankConverge->load();
    mSingleTankConvergeContent = RenderableContentRef(tankConverge);
    
    DumbTankContent *dumbTank = new DumbTankContent();
    dumbTank->load();
    mDumbTankContent = RenderableContentRef(dumbTank);
    
    TankMultiOverheadContent *multiOver = new TankMultiOverheadContent();
    multiOver->load();
    mMultiOverheadContent = RenderableContentRef(multiOver);
    
    OpponentContent *opponentContent = new OpponentContent();
    opponentContent->load();
    mOpponentContent = RenderableContentRef(opponentContent);
    
    TextLoopContent *textLoopContent = new TextLoopContent();
    textLoopContent->load();
    mTextLoopContent = RenderableContentRef(textLoopContent);
    
    StaticContent *staticContent = new StaticContent();
    staticContent->load();
    mStaticContent = RenderableContentRef(staticContent);
    
    LandscapeContent *landscapeContent = new LandscapeContent();
    landscapeContent->load("landscape_flat.png");
    mLandscapeContent = RenderableContentRef(landscapeContent);
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
#ifdef RENDER_FRAMES
    string settingsFilename = "settings."+to_string(CLIENT_ID)+".render.xml";
#else
#if IS_IAC
    string settingsFilename = "settings."+to_string(CLIENT_ID)+".IAC.xml";
#else
    string settingsFilename = "settings."+to_string(CLIENT_ID)+".xml";
#endif
#endif // ifdef
    console() << "Loading settings: " << settingsFilename << "\n";
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
// Content (Scene) Provider
RenderableContentRef BigScreensCompositeApp::contentForKey(const std::string & contentName)
{
    if (contentName == kContentKeyTankSpin ||
        contentName == kContentKeyTankOverhead ||
        contentName == kContentKeyTankWide ||
        contentName == kContentKeyTankHeightmap ||
        contentName == kContentKeyTankSideCarriage ||
        contentName == kContentKeyTankFlat )
    {
        return mTankContent;
    }
    else if (contentName == kContentKeyLandscape)
    {
        return mLandscapeContent;
    }
    else if (contentName == kContentKeyTankMultiOverhead)
    {
        return mMultiOverheadContent;
    }
    else if (contentName == kContentKeyStatic)
    {
        return mStaticContent;
    }
    else if (contentName == kContentKeyTankHorizon)
    {
        return mDumbTankContent;
    }
    else if (contentName == kContentKeyTanksConverge)
    {
        return mConvergenceContent;
    }
    else if (contentName == kContentKeySingleTankConverge)
    {
        return mSingleTankConvergeContent;
    }
    else if (contentName == kContentKeyPerlin)
    {
        return mPerlinContent;
    }
    else if (contentName == kContentKeyOpponent)
    {
        return mOpponentContent;
    }
    // Keep this last
    else if (contentName.compare(0, kContentKeyTextPrefix.length(), kContentKeyTextPrefix) == 0)
    {
        // starts with "text"...
        return mTextLoopContent;
    }

    // Default is a blank texture
    return mTextureContentBlank;
}

float * BigScreensCompositeApp::getFFTData()
{
#ifdef RENDER_FRAMES
    return (float *)kCannedFFTData[mClient->getCurrentRenderFrame()];
#else
    return mSoundtrack->getFftData();
#endif
}

float BigScreensCompositeApp::getFFTDataForChannel(const int channel)
{
    return getFFTData()[channel];
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
        else if (key == 'f')
        {
            mShouldFire = true;
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
    mSoundtrack->seekToTime(mTimeline->getPlayheadMillisec() * 0.001);
    startConvergenceClock();
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
    mSoundtrack->seekToTime(mTimeline->getPlayheadMillisec() * 0.001);
}

void BigScreensCompositeApp::reverse()
{
    console() << "reverse\n";
    mTimeline->stepToPreviousLayout();
    mSoundtrack->seekToTime(mTimeline->getPlayheadMillisec() * 0.001);
}

#pragma mark - Update

void BigScreensCompositeApp::update()
{
    if (!mClient->isConnected() && (getElapsedFrames() % 60) == 0)
    {
        mClient->start();
    }
    if (IS_IAC)
    {
        hideCursor();
    }
}

void BigScreensCompositeApp::mpeFrameUpdate(long serverFrameNumber)
{
    long long timelineMS = mTimeline->update();
    
#if IS_IAC
    if (serverFrameNumber > 1 &&
        !mTimeline->isPlaying())
    {
        // start the playback
        play();
    }
#endif
    
    // FFT Data
    // float *fftData = mSoundtrack->getFftData();
    
    // Send the controller the current frame if it's changed
    if (mLayoutIndex != mTimeline->getCurrentFrame())
    {
        mLayoutIndex = mTimeline->getCurrentFrame();
        newLayoutWasSet();

        startConvergenceClock();
    }
    
    // Update the convergence duration
    if (mMSConvergenceBegan > 0)
    {
        mMSElapsedConvergence = timelineMS - mMSConvergenceBegan;
    }
    
    // Slowly increase the explosion scale at the end to fade out
    mScalarTimelineProgress = (double)mTimeline->getPlayheadMillisec() / (double)kMSFullPlayDuration;
    const static float kAmtScaleExplosion = 0.75f;
    float fadeOutAmt = std::max<float>(0.0, mScalarTimelineProgress - (1.0-kAmtScaleExplosion)) / kAmtScaleExplosion;
    if (fadeOutAmt > 0)
    {
        MaxExplosionScale = kDefaultExplosionScale + (fadeOutAmt * kDefaultExplosionScale * 10.0f);
    }
    else
    {
        MaxExplosionScale = kDefaultExplosionScale;
    }
}

void BigScreensCompositeApp::startConvergenceClock()
{
    if (mLayoutIndex == mPreConvergenceLayoutIndex)
    {
        long long timelineMS = mTimeline->getPlayheadMillisec();
        
        // If this is the layout that starts the convergence,
        // make a note of the time.
        if (mLayoutIndex == mPreConvergenceLayoutIndex)
        {
            // reset
            console() << "STARTING CONVERGENCE CLOCK\n";
            mMSElapsedConvergence = 0;
            mMSConvergenceBegan = timelineMS;
        }
    }
}

const static int kChanceFire = 100;

void BigScreensCompositeApp::updateContentForRender(const TimelineContentInfo & contentInfo,
                                                    const int contentID)
{
    long long contentElapsedFrames = contentInfo.numRenderFrames;
    
    // Arbitrary
    Vec3f tankPosition(0, 0, contentElapsedFrames * 10);
    
    RenderableContentRef content = contentInfo.contentRef;
    content->setFramesRendered(contentElapsedFrames);
    content->setFrameContentID(contentID);
    
    // TODO: Use blink spin / dumbTank update
    if (contentInfo.contentKey == kContentKeyTankSpin) // TMP
    {
        // Blinking tank rotation
        float tankRotation = contentElapsedFrames * 0.01;
        shared_ptr<TankContent> scene = static_pointer_cast<TankContent>(content);
        scene->resetPositions();
        scene->setDefaultGroundScale();
        scene->setGroundIsVisible(true);
        
        // Center / still
        scene->setTankPosition(Vec3f::zero());
        // Wheel speed has to happen before update
        scene->getTank()->setWheelSpeedMultiplier(0);

        scene->update([=](CameraPersp & cam, AdvancedTankRef & tank)
        {
            if (mShouldFire || ((int)arc4random() % kChanceFire == 1) ) scene->fireTankGun();
            
            float camX = cosf(tankRotation) * 1000;
            float camZ = sinf(tankRotation) * 1000;
            cam.lookAt(Vec3f(camX,
                             1000, // TMP
                             camZ),
                       Vec3f(0,
                             kTankBodyCenterY + 300,
                             0));
        });
    }
    else if (contentInfo.contentKey == kContentKeyStatic)
    {
        shared_ptr<StaticContent> scene = static_pointer_cast<StaticContent>(content);
        // scene->update();
    }
    else if (contentInfo.contentKey == kContentKeyTankOverhead)
    {
        // Flat texture ground
        float tankDistance = sinf(contentElapsedFrames * 0.0025);
        shared_ptr<TankContent> scene = static_pointer_cast<TankContent>(content);
        scene->resetPositions();
        scene->setDefaultGroundScale();
        // scene->setGroundOffset(tankGroundOffset);
        scene->setGroundIsVisible(true);
        scene->setTankPosition(tankPosition);
        // Wheel speed has to happen before update
        scene->getTank()->setWheelSpeedMultiplier(6);

        scene->update([=](CameraPersp & cam, AdvancedTankRef & tank)
        {
            // Zoom in and out

            if (mShouldFire || ((int)arc4random() % kChanceFire == 1) ) scene->fireTankGun();
            
            float camZ = tankPosition.z + (tankDistance * 1000);
            cam.lookAt(Vec3f(tankPosition.x + 100,
                             kTankBodyCenterY * 8,
                             camZ),
                       Vec3f(tankPosition.x,
                             tankPosition.y + kTankBodyCenterY,
                             tankPosition.z));
        });
        
    }
    else if (contentInfo.contentKey == kContentKeyTankWide)
    {
        // Tank content wide shot
        shared_ptr<TankContent> scene = static_pointer_cast<TankContent>(content);
        scene->resetPositions();
        scene->setDefaultGroundScale();
        scene->setGroundIsVisible(true);
        scene->setTankPosition(tankPosition);
        // Wheel speed has to happen before update
        scene->getTank()->setWheelSpeedMultiplier(6);

        scene->update([=](CameraPersp & cam, AdvancedTankRef & tank)
        {
            if (mShouldFire || ((int)arc4random() % kChanceFire == 1) ) scene->fireTankGun();

            float camX, camY, camZ;
            switch (CLIENT_ID)
            {
                case 0:
                    camY = kTankBodyCenterY * 2;
                    camZ = 10000;
                    camX = -500 + (contentElapsedFrames * -0.5);
                    break;
                case 1:
                    camY = kTankBodyCenterY * 2;
                    camZ = 3000 + (contentElapsedFrames * 5.0);
                    camX = 500 + (1000 - contentElapsedFrames);
                    break;
                case 2:
                    camY = kTankBodyCenterY * 2;
                    camZ = -3000 + (contentElapsedFrames * 16);
                    camX = 1500 + (camZ*-0.15);
                    break;
            }
            
            Vec3f eyePos(camX, camY, camZ);
            Vec3f taregtPos(tankPosition.x, tankPosition.y + kTankBodyCenterY, tankPosition.z);
            float distToTarget = eyePos.distance(taregtPos);
            
            eyePos.y += distToTarget * 0.1;
            
            cam.lookAt(eyePos, taregtPos);
        });
    }
    else if (contentInfo.contentKey == kContentKeyTankMultiOverhead)
    {
        shared_ptr<TankMultiOverheadContent> scene = static_pointer_cast<TankMultiOverheadContent>(content);
        
        scene->resetPositions();
        scene->setDefaultGroundScale();
        scene->setGroundIsVisible(false);
        // ?
        // Vec2f masterSize = mClient->getMasterSize();
        // float fullAspectRatio = masterSize.x / masterSize.y;
        scene->update([=](CameraPersp & cam, DumbTankRef & tank)
                      {
                          float camX = 0;
                          float camY = 80000;
                          float camZ = 0;
                          cam.setPerspective(5, getWindowAspectRatio(), 0.01, 150000);
                          // cam.setAspectRatio(fullAspectRatio);
                          cam.lookAt(Vec3f(camX,camY,camZ),
                                     Vec3f(0,0,0));
                      });
    }
    else if (contentInfo.contentKey == kContentKeyOpponent)
    {
        shared_ptr<OpponentContent> scene = static_pointer_cast<OpponentContent>(content);
        scene->update([=](CameraPersp & cam, OpponentRef & opponent)
        {
            float progress = contentElapsedFrames / 800.0;
            Vec3f eye(0,0,0);
            Vec3f target(0,0,0);
            switch (CLIENT_ID)
            {
                case 0:
                    // Circle above
                    eye.x = cos(progress * M_PI);
                    eye.y = kOpponentScale;
                    eye.z = sin(progress * M_PI);
                    break;
                case 1:
                    // Drop
                    eye.x = cos(progress * M_PI * 0.25) * (kOpponentScale * -2);
                    eye.y = kOpponentScale * 2 * (1.5-progress);
                    target.y = eye.y;
                    eye.z = sin(progress * M_PI * 0.25) * (kOpponentScale * -2);
                    break;
                case 2:
                    // Zoom way out to way in
                    eye.x = 0;
                    eye.y = kOpponentScale * 0.33;
                    target.y = eye.y;
                    eye.z = (kOpponentScale * -6) + ((kOpponentScale * 2) * progress);
                    break;
            }
            cam.lookAt(eye, target);
        });
    }
    else if (contentInfo.contentKey == kContentKeyTankHorizon)
    {
        // Tank content horizon shot
        shared_ptr<DumbTankContent> scene = static_pointer_cast<DumbTankContent>(content);

        scene->resetPositions();
        scene->setDefaultGroundScale();
        scene->setTankPosition(Vec3f(0, 0, -45000 * (1.0-(contentElapsedFrames/4000.0))));
        scene->setGroundIsVisible(false);

        scene->update([=](CameraPersp & cam, DumbTankRef & tank)
        {
            if (mShouldFire) scene->fireTankGun();
            
            cam.setPerspective(5, getWindowAspectRatio(), 0.01, 150000);
            const float camX = -80000;
            const float camY = 100;
            const float camZ = 0;
            cam.lookAt(Vec3f( camX, camY, camZ ),
                       Vec3f( 0, camY, camZ ) );
        });        
    }
    else if (contentInfo.contentKey == kContentKeyTanksConverge)
    {
        shared_ptr<ConvergenceContent> scene = static_pointer_cast<ConvergenceContent>(content);
        scene->setMSElapsed(mMSElapsedConvergence);
        scene->update(mScalarTimelineProgress);
    }
    else if (contentInfo.contentKey == kContentKeySingleTankConverge)
    {
        shared_ptr<TankConvergenceContent> scene = static_pointer_cast<TankConvergenceContent>(content);
        
        // ! Don't reset
        // scene->resetPositions();
        
        scene->setMSElapsed(mMSElapsedConvergence);
        //mSingleTankConvergeContent
        // Nothing to see here

        int i = contentInfo.layoutIndex; // this is fine
        int regionCount = mTimeline->getCurrentRegionCount();
        Vec2f masterSize = mClient->getMasterSize();

        CameraOrigin orig = TankConvergenceContent::cameraForTankConvergence(i,
                                                                             regionCount,
                                                                             mMSElapsedConvergence,
                                                                             masterSize,
                                                                             contentInfo.rect);
        float fullAspectRatio = masterSize.x / masterSize.y;
        
        scene->update([=](CameraPersp & cam, DumbTankRef & tank)
                      {                          
                          if (mShouldFire) scene->fireTankGun();
                          
                          cam.setAspectRatio(fullAspectRatio);
                          cam.lookAt( orig.eye, orig.target );
                          cam.setLensShift(orig.camShift);
                      });
    }
    else if (contentInfo.contentKey == kContentKeyPerlin)
    {
        // Perlin
        // NOTE: The movement here doesn't map to PX.
        // The image /should/ shift 1px per frame...
        shared_ptr<PerlinContent> scene = static_pointer_cast<PerlinContent>(content);
        scene->update(Vec2f(0.0, -0.2));
    }
    else if (contentInfo.contentKey == kContentKeyTankHeightmap)
    {
        // Heightmap
        shared_ptr<TankContent> scene = static_pointer_cast<TankContent>(content);
        scene->resetPositions();
        
        Vec3f tankPosition(contentElapsedFrames * 5, 0, contentElapsedFrames * 10);
        float theta = atan2(tankPosition.x, tankPosition.z);
        scene->setTankPosition(tankPosition, theta);
        
        // Wheel speed has to happen before update
        scene->getTank()->setWheelSpeedMultiplier(6);
        scene->update([=](CameraPersp & cam, AdvancedTankRef & tank)
        {
            if (mShouldFire || ((int)arc4random() % kChanceFire == 1) ) scene->fireTankGun();
            
            cam.lookAt(Vec3f( 0, 1200, -1000 ) + tankPosition,
                       Vec3f( 0, 100, 0 ) + tankPosition);
        });

    }
    else if (contentInfo.contentKey == kContentKeyTankFlat)
    {
        // Heightmap
        shared_ptr<TankContent> scene = static_pointer_cast<TankContent>(content);
        scene->resetPositions();
        scene->setGroundScale(Vec3f(10000,1,10000));
        
        Vec3f tankPosition(0, 0, contentElapsedFrames * 100);
        scene->setTankPosition(tankPosition);
        
        // Wheel speed has to happen before update
        scene->getTank()->setWheelSpeedMultiplier(6);
        scene->update([=](CameraPersp & cam, AdvancedTankRef & tank)
                      {
                          if (mShouldFire || ((int)arc4random() % kChanceFire == 1) ) scene->fireTankGun();
                          
                          cam.lookAt(tankPosition + Vec3f( 0, kTankBodyCenterY, 2000 ),
                                     tankPosition + Vec3f( 0, kTankBodyCenterY, 0 ));
                      });
        
    }
    else if (contentInfo.contentKey == kContentKeyTankSideCarriage)
    {
        shared_ptr<TankContent> scene = static_pointer_cast<TankContent>(content);
        scene->resetPositions();

        Vec3f tankPosition(contentElapsedFrames * 10,
                           0,
                           contentElapsedFrames * 15);
        float theta = atan2(tankPosition.x, tankPosition.z);
        scene->setTankPosition(tankPosition, theta);
        
        // Vec3f normalDirection = tankPosition.normalized();
        
        Vec3f eye(1500 + (cos(contentElapsedFrames * 0.01) * 800),
                  500,// + (sin(contentElapsedFrames * 0.05) * 200),
                  -400 );
        eye.rotateY(-theta);
        
        Vec3f lookAt( 1000, 300, 10000);
        lookAt.rotateY(theta);
        
        // Wheel speed has to happen before update
        scene->getTank()->setWheelSpeedMultiplier(4);
        scene->update([=](CameraPersp & cam, AdvancedTankRef & tank)
                      {
                          if (mShouldFire || ((int)arc4random() % kChanceFire == 1) ) scene->fireTankGun();
                          cam.lookAt(tankPosition + eye,
                                     tankPosition + lookAt);
                      });
        
    }
    else if (contentInfo.contentKey == kContentKeyLandscape)
    {
        // Landscape
        shared_ptr<LandscapeContent> scene = static_pointer_cast<LandscapeContent>(content);
        int xDir = (contentID % 3) - 1;
        float xVec = xDir * 0.1;
        scene->setScrollVector(Vec2f(xVec, -0.5f));
        float offsetX = 0;
        if (xVec < 0)
        {
            offsetX = -256.0f;
        }
        scene->setInitialOffset(Vec2f(offsetX,-50 * contentID));
    }
    else if (contentInfo.contentKey.compare(0, kContentKeyTextPrefix.length(), kContentKeyTextPrefix) == 0)
    {
        // It's a text module.
        shared_ptr<TextLoopContent> scene = static_pointer_cast<TextLoopContent>(content);
        // This is where we set the text.
        // Even though it's set every frame, the content is cached so we
        // don't re-create the texture unless it's not there.
        /*
        if (!scene->hasTextForContentID(contentID))
        {
            TextContentProvider::TextTimelineAndHeight timeAndHeight =
                TextContentProvider::textTimelineForContentKey(contentInfo.contentKey);
            scene->setTextForContentID(timeAndHeight.timeline,
                                       contentID,
                                       timeAndHeight.absoluteLineHeight * kScreenScale);
        }
        */
        
        scene->update();
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
    bool shouldTransition = mTimeline->getCurrentFrame() != mConvergenceLayoutIndex;
    
    std::map<int, TimelineContentInfo > renderContent = mTimeline->getRenderContent(//this,
                                                                                    shouldTransition);
    
    Rectf clientRect = mClient->getVisibleRect();
    Vec2i screenOffset = clientRect.getUpperLeft();
    Vec2f masterSize = mClient->getMasterSize();
    
	mFbo->bindFramebuffer();
    gl::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

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
            ci::gl::enable( GL_SCISSOR_TEST );

            // NOTE: Do the updates >>> here <<<
            // Some module won't be rendered (and therefor shouldn't be updated)
            // and other modules will be rendered (and updated) more than once.
            
            updateContentForRender(renderMe, contentID);
            
            // NOTE: I removed SceneWindow for 2 reasons:
            // 1) We can pull the same behavior into the app w/out having to allocate memory for each render.
            // 2) Not all content behaves the same way (esp. convergence) w.r.t. viewport/scissor/aspect.
            
            Rectf contentRect = renderMe.rect;
            OriginAndDimension contentOrigAndDim = OriginAndDimensionFromRectf(contentRect, masterSize.y);
            RenderableContentRef content = renderMe.contentRef;

            ci::gl::scissor(contentOrigAndDim.first.x - screenOffset.x,
                            contentOrigAndDim.first.y - screenOffset.y,
                            contentOrigAndDim.second.x,
                            contentOrigAndDim.second.y);
            
            if (renderMe.contentKey != kContentKeySingleTankConverge)
            {
                // Ported from SceneWindow
                content->getCamera().setAspectRatio( (float)contentRect.getWidth() / contentRect.getHeight() );

                ci::gl::viewport(contentOrigAndDim.first.x - screenOffset.x,
                                 contentOrigAndDim.first.y - screenOffset.y,
                                 contentOrigAndDim.second.x,
                                 contentOrigAndDim.second.y );
                
                content->render(screenOffset, renderMe.rect);
            }
            else
            {
                // The SingleTankConverge needs to be handled differently since the viewport
                // extends beyond the box.
                content->getCamera().setAspectRatio( (float)masterSize.x / masterSize.y );

                // The viewport is the entire screen (e.g. all screens combined)
                ci::gl::viewport(-screenOffset.x,
                                 -screenOffset.y,
                                 masterSize.x,
                                 masterSize.y);

                content->render(screenOffset, renderMe.rect);

                // Reset the viewport for the outline
                ci::gl::viewport(contentOrigAndDim.first.x - screenOffset.x,
                                 contentOrigAndDim.first.y - screenOffset.y,
                                 contentOrigAndDim.second.x,
                                 contentOrigAndDim.second.y);

            }

            if (content->drawsOutline())
            {
                // NOTE: Convergence takes care of it's own outlines
                if (renderMe.contentKey != kContentKeyTanksConverge)
                {
                    mOutLine->render();
#if IS_IAC
                    // Drawing a second, inner line at IAC because we're missing some edges
                    ci::gl::viewport(contentOrigAndDim.first.x - screenOffset.x + 1,
                                     contentOrigAndDim.first.y - screenOffset.y + 1,
                                     contentOrigAndDim.second.x - 2,
                                     contentOrigAndDim.second.y - 2);
                    mOutLine->render();
#endif
                }
            }
            
            ci::gl::disable( GL_SCISSOR_TEST );
        }
    }
    
    mFbo->unbindFramebuffer();
	mFinalBillboard->draw( mFbo->getTexture() );
	
    if (mIsDrawingColumns)
    {
        renderColumns();
    }
    
    mCurrentContentInfo = newContentInfo;
    
    mShouldFire = false;
    
#ifdef RENDER_FRAMES
    {
        gl::Texture & fboTex = *mFbo->getTexture();
        Surface flipped(fboTex);
        ci::ip::flipVertical(&flipped);
        ci::writeImage(getHomeDirectory() / "Documents" / "BigScreensRender" / std::to_string(CLIENT_ID) / (std::to_string(mClient->getCurrentRenderFrame()) + ".png"), flipped);
    }
#endif
    
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

void BigScreensCompositeApp::newLayoutWasSet()
{
    shared_ptr<TextLoopContent> textScene = static_pointer_cast<TextLoopContent>(mTextLoopContent);
    textScene->newLayoutWasSet(mTimeline->getCurrentLayout());
    
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
