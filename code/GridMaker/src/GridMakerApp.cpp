#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Rect.h"
#include "cinder/gl/Texture.h"
#include "cinder/Utilities.h"
#include "Utilities.hpp"
#include "cinder/Camera.h"
#include "GridLayout.h"
#include "TankContent.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace bigscreens;

const int kGridSnapSize = 5;

class GridMakerApp : public AppNative {
  public:
    
    // Setup
    void prepareSettings(Settings *settings);
	void setup();
	void shutdown();

    // Load / Save
    void loadAllGrids();
    void save();
    void reload();

    // Mouse
	void mouseDown( MouseEvent event );
    void mouseUp(MouseEvent event);
    void mouseDrag(MouseEvent event);
    void mouseMove(MouseEvent event);
    
    // Key
    void keyDown(KeyEvent event);
    void keyUp(KeyEvent event);
    
    // Button
    void saveContentPressed();
    
    // Playback
    void advance();
    void reverse();
    void restart();
    void play();
    void pause();
    void newLayoutWasSet();
    
    // App Loop
    void update();
    void draw();
    void renderLayout(const GridLayout & layout,
                      const GridLayout & otherLayout,
                      const float transitionAmount,
                      const Vec2f & mousePosition,
                      const GridRenderMode mode,
                      RenderableContentRef & content);
    void drawPillars();
    void drawMargins();

    // Grid Editing
    void clearSelection();
    void insertNewLayout();
    void insertLayout(GridLayout newLayout);
    void splitScreen();
    void finishJoining(MouseEvent event);
    void finishRemoving(MouseEvent event);
    void finishAdding(MouseEvent event);
    void finishRegionSelection(MouseEvent event);
    void deleteCurrentLayout();
    void clearCurrentLayout();
    void duplicateCurrentLayout();
    
    // Time Editing
    void adjustLayoutTimestamps(int startingIndex, long long duration);

    // Misc
    void calculateTotalDuration();
    
    // Member Vars
    vector<GridLayout> mGridLayouts;
    int mIdxCurrentLayout;
    int mIdxPrevLayout;
    
    // Editing
    int mSelectedRegionIndex;
    bool mIsAdding;
    bool mIsRemoving;
    bool mIsJoining;
    bool mIsMouseValid;
    Vec2i mMousePositionStart;
    Vec2i mMousePositionEnd;
    Vec2i mMousePosition;
    Rectf mDrawingRect;

    // Timing
    float mTransitionAmt;
    long long mStartTime;
    long long mPlayheadTime;
    long long mLastFrameTime;
    long long mTotalDuration;
    float mPlaybackSpeed;
    bool mIsPlaying;

    // Sample content
    ci::gl::Texture mScreenTexture;
    //TankContent mContent;
    RenderableContentRef mTankContent;
    
    // Playback indicators
    ci::gl::Texture mTexturePlaying;
    ci::gl::Texture mTexturePaused;
    
    // size of the wrap
    ci::Vec2i mGridWrap;
    
    // Params
    ci::params::InterfaceGlRef mParams;
    string mContentName;
    string mGridName;
    
    bool mDrawPillars;
};

#pragma mark - Setup

const static int kNumScreens = 3;
const static int kScreenWidth = 3840;
const static int kScreenHeight = 1080;
const static int kScreenMarginBottom = 133;
const static int kScreenMarginRight = 147; // Estimate
const static float kScreenScale = 0.25f;

void GridMakerApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(kScreenWidth * kScreenScale,
                            kScreenHeight * kScreenScale * kNumScreens);
}

void GridMakerApp::setup()
{
    mGridWrap = Vec2i(kScreenWidth, kScreenHeight);

    reload();

    mScreenTexture = loadImage(app::loadResource("screen.png"));
    mTexturePlaying = loadImage(app::loadResource("playing.png"));
    mTexturePaused = loadImage(app::loadResource("paused.png"));

    TankContent *tank = new TankContent();
    tank->load("T72.obj");
    mTankContent = RenderableContentRef(tank);
    //mContent.load("T72.obj");
    
    mDrawPillars = true;
    
    // Using params for content input.
    // NOTE: The text field is pretty slow to update (1 second or two),
    // but this seems like the best option for the moment.
    mParams = params::InterfaceGl::create(getWindow(), "Content", Vec2i( 250, 330 ));
    mParams->addParam( "Grid: ", &mGridName, "" );
    mParams->addSeparator();
    mParams->addButton( "Save Layouts", std::bind( &GridMakerApp::save, this ) );
    mParams->addButton( "Reload", std::bind( &GridMakerApp::reload, this ) );
    mParams->addSeparator();
    mParams->addParam( "Region Content: ", &mContentName, "" );
    mParams->addButton( "Save Region Content", std::bind( &GridMakerApp::saveContentPressed, this ) );
    mParams->addSeparator();
    mParams->addButton( "Delete Layout", std::bind( &GridMakerApp::deleteCurrentLayout, this ) );
    mParams->addButton( "Insert Layout", std::bind( &GridMakerApp::insertNewLayout, this ) );
    mParams->addButton( "Duplicate Layout", std::bind( &GridMakerApp::duplicateCurrentLayout, this ) );
    mParams->addSeparator();
    mParams->addButton( "Play", std::bind( &GridMakerApp::play, this ) );
    mParams->addButton( "Pause", std::bind( &GridMakerApp::pause, this ) );
    mParams->addButton( "Restart", std::bind( &GridMakerApp::restart, this ) );
    mParams->addButton( "Advance", std::bind( &GridMakerApp::advance, this ) );
    mParams->addButton( "Reverse", std::bind( &GridMakerApp::reverse, this ) );
    mParams->addSeparator();
    mParams->addParam( "Draw Pillars", &mDrawPillars);
}

void GridMakerApp::shutdown()
{
}

#pragma mark - File Management

void GridMakerApp::save()
{
    console() << "Saving\n";
    
    fs::path gridPath = SharedGridAssetPath(true);
    
    for (int i = 0; i < mGridLayouts.size(); ++i)
    {
        GridLayout & layout = mGridLayouts[i];
        if (layout.getRegions().size() > 0)
        {
            layout.serialize(gridPath, kScreenScale, mGridWrap);
        }
        else
        {
            layout.remove();
        }
    }
}

void GridMakerApp::reload()
{
    mGridLayouts.clear();
    mIsAdding = false;
    mIsRemoving = false;
    mIsMouseValid = true;
    mIsJoining = false;
    mIsPlaying = false;
    mSelectedRegionIndex = -1;

    // playback
    mIdxCurrentLayout = -1;
    mIdxPrevLayout = -1;
    mTransitionAmt = 1.0f;
    mPlaybackSpeed = 1.0f;
    mTotalDuration = 0;
    mStartTime = 0;
    mPlayheadTime = 0;
    mLastFrameTime = getMilliCount();

    loadAllGrids();
    
    if (mIdxCurrentLayout > -1)
    {
        splitScreen();
    }
    else
    {
        console() << "Didn't find any serialized grids." << endl;
        // Add an empty layout
        GridLayout newLayout;
        newLayout.setTimestamp(0);
        newLayout.setTransitionDuration(kDefaultTransitionDuration);
        
        mGridLayouts.push_back(newLayout);
        
        mIdxCurrentLayout = 0;
    }
    
    newLayoutWasSet();
    calculateTotalDuration();
}

void GridMakerApp::loadAllGrids()
{
    fs::path gridPath = SharedGridAssetPath(true);
    mGridLayouts = GridLayout::loadAllFromPath(gridPath, kScreenScale, mGridWrap);
    int numLayouts = mGridLayouts.size();
    if (numLayouts > 0)
    {
        mIdxCurrentLayout = 0;
    }
}

#pragma mark - Mouse Input

void GridMakerApp::mouseDown( MouseEvent event )
{
    mMousePositionStart = mousePositionSnappedToSize(event.getPos(), kGridSnapSize);
    mDrawingRect = Rectf(mMousePosition.x, mMousePosition.y,
                              mMousePosition.x, mMousePosition.y);
}

void GridMakerApp::mouseDrag(MouseEvent event)
{
    mMousePosition = mousePositionSnappedToSize(event.getPos(), kGridSnapSize);
    
    if (mIsAdding)
    {
        mDrawingRect = rectFromTwoPos(mMousePosition, mMousePositionStart);
        // +1 to avoid snap-to-grid collisions
        mDrawingRect.x1 += 1;
        mDrawingRect.y1 += 1;
    }
    else
    {
        // +1 to avoid snap-to-grid collisions
        mDrawingRect = Rectf(mMousePosition.x + 1, mMousePosition.y + 1,
                                  mMousePosition.x + 1, mMousePosition.y + 1);
    }

    GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
    mIsMouseValid = rectIsValid(mDrawingRect, curLayout.getRegions());
    
    if (mIsMouseValid)
    {
        mMousePositionEnd = mMousePosition;
    }
}

void GridMakerApp::mouseMove(MouseEvent event)
{
    mMousePosition = mousePositionSnappedToSize(event.getPos(), kGridSnapSize);
    // +1 to avoid snap-to-grid collisions
    mDrawingRect = Rectf(mMousePosition.x + 1, mMousePosition.y + 1,
                              mMousePosition.x + 1, mMousePosition.y + 1);
    GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
    mIsMouseValid = rectIsValid(mDrawingRect, curLayout.getRegions());
}

void GridMakerApp::mouseUp(MouseEvent event)
{
    if (mIsAdding)
    {
        clearSelection();
        finishAdding(event);
    }
    else if (mIsRemoving)
    {
        clearSelection();
        finishRemoving(event);
    }
    else if(mIsJoining)
    {
        finishJoining(event);
    }
    else
    {
        finishRegionSelection(event);
    }
}

#pragma mark - Key Input

void GridMakerApp::keyDown(KeyEvent event)
{
    mIsAdding = event.isShiftDown();
    if (mIsAdding) console() << "ADDING" << endl;
    mIsJoining = event.isControlDown();
    if (mIsJoining) console() << "JOINING" << endl;
    mIsRemoving = event.isAltDown();
    if (mIsRemoving) console() << "REMOVING" << endl;
}

void GridMakerApp::keyUp(KeyEvent event)
{
    
    mIsAdding = event.isShiftDown();
    
    bool isJoining = event.isControlDown();

    mIsJoining = isJoining;
    
    mIsRemoving = event.isAltDown();
    
    if (mIsAdding || mIsJoining || mIsRemoving)
    {
        // Don't allow playback while editing
        pause();
    }
    
    if (event.getCode() == KeyEvent::KEY_RIGHT)
    {
        advance();
    }
    else if (event.getCode() == KeyEvent::KEY_LEFT)
    {
        reverse();
    }
}

#pragma mark - Button Input

void GridMakerApp::saveContentPressed()
{
    if (mIdxCurrentLayout != -1 && mSelectedRegionIndex != -1)
    {
        GridLayout &layout = mGridLayouts[mIdxCurrentLayout];
        vector<ScreenRegion> regions = layout.getRegions();
        ScreenRegion &reg = regions[mSelectedRegionIndex];
        reg.contentKey = mContentName;
        // Update the content
        layout.setRegions(regions);
    }
}

#pragma mark - Playback

void GridMakerApp::restart()
{
    mStartTime = 0;
    mIdxCurrentLayout = 0;
    mTransitionAmt = 1.0f;
    mPlayheadTime = 0;
//    play();
}

void GridMakerApp::play()
{
    mLastFrameTime = getMilliCount();
    mIsPlaying = true;
}

void GridMakerApp::pause()
{
    mIsPlaying = false;
}

void GridMakerApp::advance()
{
    clearSelection();
    mIdxPrevLayout = mIdxCurrentLayout;
    mIdxCurrentLayout = (mIdxCurrentLayout + 1) % mGridLayouts.size();
    newLayoutWasSet();
}

void GridMakerApp::reverse()
{
    clearSelection();
    mIdxPrevLayout = mIdxCurrentLayout;
    mIdxCurrentLayout = mIdxCurrentLayout - 1;
    if (mIdxCurrentLayout < 0)
    {
        mIdxCurrentLayout = mGridLayouts.size() - 1;
    }
    newLayoutWasSet();
}

void GridMakerApp::newLayoutWasSet()
{
    mTransitionAmt = mIsPlaying ? 0.0f : 1.0f;
    
    // Always reset the playhead so if we fwd/bck while playing it
    // continues from the current frame.
    GridLayout & layout = mGridLayouts[mIdxCurrentLayout];
    mPlayheadTime = layout.getTimestamp();
    mGridName = layout.getName();
    
    mSelectedRegionIndex = -1;

    splitScreen();
}

#pragma mark - App Loop

void GridMakerApp::update()
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
    
    static_pointer_cast<TankContent>(mTankContent)->update();
}

void GridMakerApp::draw()
{
    mTankContent->render(Vec2i::zero());
    
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    // set the viewport to match our window
	gl::setViewport( getWindowBounds() );
    gl::setMatricesWindow( getWindowSize() );

    gl::enableAlphaBlending();
    
    GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
    
    GridRenderMode mode = GridRenderModeNormal;
    if (mIsAdding)
    {
        mode = GridRenderModeAdding;
    }
    else if(mIsJoining)
    {
        mode = GridRenderModeJoining;
    }
    else if(mIsRemoving)
    {
        mode = GridRenderModeRemoving;
    }
    
    if (mTransitionAmt < 1.0 && mIdxPrevLayout != -1)
    {
        GridLayout & prevLayout = mGridLayouts[mIdxPrevLayout];

        renderLayout(prevLayout,
                     curLayout,
                     1.0 - std::min(mTransitionAmt / 0.5f, 1.0f),
                     mMousePosition,
                     mode,
                     mTankContent);
        
        renderLayout(curLayout,
                     prevLayout,
                     std::max((mTransitionAmt - 0.5f) / 0.5f, 0.0f),
                     mMousePosition,
                     mode,
                     mTankContent);
    }
    else
    {
        renderLayout(curLayout,
                     GridLayout(),
                     mTransitionAmt,
                     mMousePosition,
                     mode,
                     mTankContent);
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
    
    // Screen bounds
    gl::lineWidth(1.0f);
    gl::color(ColorAf(0.3f, 0.0f, 0.9f, 1.0f));
    float screenHeight = kScreenHeight * kScreenScale;
    for (int i = 0; i < kNumScreens; ++i)
    {
        float y = screenHeight * (i + 1);
        gl::drawLine(Vec2f(0, y),
                     Vec2f(getWindowWidth(), y));
    }

    // Draw the current frame num
    gl::drawString("Frame " + to_string(mIdxCurrentLayout), Vec2f(42, 25));

    float height = getWindowHeight();
    float width = getWindowWidth();
    
    if (mIsAdding)
    {
        gl::lineWidth(1.0f);
        
        if (mIsMouseValid)
        {
            gl::color(0.0f, 1.0f, 1.0f);
        }
        else
        {
            gl::color(1.0f, 0.0f, 0.0f);
        }
        
        // Draw guide lines
        float x1 = mDrawingRect.getX1();
        float x2 = mDrawingRect.getX2();
        float y1 = mDrawingRect.getY1();
        float y2 = mDrawingRect.getY2();
        
        gl::drawLine(Vec2f(x1, 0), Vec2f(x1, height));
        gl::drawLine(Vec2f(x2, 0), Vec2f(x2, height));
        gl::drawLine(Vec2f(0, y1), Vec2f(width, y1));
        gl::drawLine(Vec2f(0, y2), Vec2f(width, y2));
        
        gl::color(1.0f, 1.0f, 1.0f);
        gl::drawStrokedRect(mDrawingRect);
        
        gl::drawString(to_string((int)mDrawingRect.getWidth()) + " x " +
                       to_string((int)mDrawingRect.getHeight()),
                       mMousePosition);
    }
    
    if (!mIsAdding && !mIsJoining && !mIsRemoving)
    {
        if (mDrawPillars)
        {
            drawMargins();
            drawPillars();
        }
        mParams->draw();
    }
    
}

void GridMakerApp::drawPillars()
{
    const static float pxPerFoot = (float)((kScreenWidth * kNumScreens) - kScreenMarginRight) / 120.0f;
    const static float pillarWidth = 3.0f * pxPerFoot;
    const static float posPillar1 = 3.5f * pxPerFoot;
    const static float posPillar2 = (3.5f + 33.166f) * pxPerFoot;
    const static float posPillar3 = (3.5f + 33.166f + 31.8333f) * pxPerFoot;
    const static float posPillar4 = (3.5f + 33.166f + 31.8333f + 33.33333f) * pxPerFoot;
    
    float x = ((int)posPillar1 % (int)mGridWrap.x) * kScreenScale;
    float x1 = x - (pillarWidth * 0.5f * kScreenScale);
    float x2 = x + (pillarWidth * 0.5f * kScreenScale);
    float row = ((int)posPillar1 / (int)mGridWrap.x);
    float y1 = row * mGridWrap.y * kScreenScale;
    float y2 = y1 + (mGridWrap.y * kScreenScale);
    
    gl::color(ColorAf(0.95,0.95,0.9));
    gl::drawSolidRect(Rectf(x1,y1,x2,y2));

    x = ((int)posPillar2 % (int)mGridWrap.x) * kScreenScale;
    x1 = x - (pillarWidth * 0.5f * kScreenScale);
    x2 = x + (pillarWidth * 0.5f * kScreenScale);
    row = ((int)posPillar2 / (int)mGridWrap.x);
    y1 = row * mGridWrap.y * kScreenScale;
    y2 = y1 + (mGridWrap.y * kScreenScale);
    
    gl::drawSolidRect(Rectf(x1,y1,x2,y2));

    x = ((int)posPillar3 % (int)mGridWrap.x) * kScreenScale;
    x1 = x - (pillarWidth * 0.5f * kScreenScale);
    x2 = x + (pillarWidth * 0.5f * kScreenScale);
    row = ((int)posPillar3 / (int)mGridWrap.x);
    y1 = row * mGridWrap.y * kScreenScale;
    y2 = y1 + (mGridWrap.y * kScreenScale);
    
    gl::drawSolidRect(Rectf(x1,y1,x2,y2));
    
    x = ((int)posPillar4 % (int)mGridWrap.x) * kScreenScale;
    x1 = x - (pillarWidth * 0.5f * kScreenScale);
    x2 = x + (pillarWidth * 0.5f * kScreenScale);
    row = ((int)posPillar4 / (int)mGridWrap.x);
    y1 = row * mGridWrap.y * kScreenScale;
    y2 = y1 + (mGridWrap.y * kScreenScale);
    
    gl::drawSolidRect(Rectf(x1,y1,x2,y2));

}

void GridMakerApp::drawMargins()
{
    gl::color(ColorAf(0.95,0.95,0.9));
    const static float visibleScreenHeight = kScreenHeight - kScreenMarginBottom;
    gl::drawSolidRect(Rectf(0,
                            visibleScreenHeight * kScreenScale,
                            kScreenWidth * kScreenScale,
                            kScreenHeight * kScreenScale));

    gl::drawSolidRect(Rectf(0,
                            (kScreenHeight + visibleScreenHeight) * kScreenScale,
                            kScreenWidth * kScreenScale,
                            kScreenHeight * 2 * kScreenScale));

    gl::drawSolidRect(Rectf(0,
                            ((kScreenHeight * 2) + visibleScreenHeight) * kScreenScale,
                            kScreenWidth * kScreenScale,
                            kScreenHeight * 3 * kScreenScale));

    gl::drawSolidRect(Rectf((kScreenWidth - kScreenMarginRight) * kScreenScale,
                            kScreenHeight * 2 * kScreenScale,
                            kScreenWidth * kScreenScale,
                            kScreenHeight * 3 * kScreenScale));
}

void GridMakerApp::renderLayout(const GridLayout & layout,
                                const GridLayout & otherLayout,
                                const float transitionAmount,
                                const Vec2f & mousePosition,
                                const GridRenderMode mode,
                                RenderableContentRef & content)
{
    vector<ScreenRegion> regions = layout.getRegions();
    int regionSize = regions.size();
    
    vector<ScreenRegion> compareRegions = otherLayout.getRegions();
    int compareRegionSize = compareRegions.size();
    
    for (int i = 0; i < regionSize; ++i)
    {
        ScreenRegion & reg = regions[i];
        
        bool isHovered = reg.rect.contains(mousePosition);
        bool isJoining = ((mode == GridRenderModeJoining) && isHovered);
        bool isRemoving = ((mode == GridRenderModeRemoving) && isHovered);
        
        float alpha = transitionAmount;
        
        Rectf rA = reg.rect;
        
        if(reg.isActive)
        {
            if (transitionAmount < 1.0f)
            {
                // Check if this is a persistant region
                for (int j = 0; j < compareRegionSize; ++j)
                {
                    ScreenRegion compareReg = compareRegions[j];
                    
                    Rectf rB = compareReg.rect;
                    
                    if (compareReg.isActive && rectCompare(rA, rB))
                    {
                        // No transition. Just draw at full blast.
                        alpha = 1.0f;
                        break;
                    }
                }
            }
            ColorAf transColor = reg.color;
            transColor[3] = alpha;
            gl::color(transColor);
            
            mScreenTexture.enableAndBind();
            
            gl::pushMatrices();
            gl::translate(rA.getCenter());
            gl::scale(alpha, alpha, 1.0f);
            
            Rectf scaledRect(rA.getWidth() * -0.5f,
                             rA.getHeight() * -0.5f,
                             rA.getWidth() * 0.5f,
                             rA.getHeight() * 0.5f);
            gl::drawSolidRect(scaledRect);
            

            gl::Texture contentTexture = static_pointer_cast<TankContent>(content)->getTexture();
            Rectf tankBounds = contentTexture.getBounds();
            
            // "Get cetered fill"
            float aspectWidth = scaledRect.getWidth() / tankBounds.getWidth();
            float aspectHeight = scaledRect.getHeight() / tankBounds.getHeight();
            float scaleFactor = std::max<float>(aspectWidth, aspectHeight);
            Vec2f drawSize = scaledRect.getSize();
            float areaMarginX = drawSize.x > drawSize.y ? 0 : ((drawSize.y - drawSize.x) * 0.5f);
            float areaMarginY = drawSize.x > drawSize.y ? ((drawSize.x - drawSize.y) * 0.5f) : 0;
            areaMarginX /= scaleFactor;
            areaMarginY /= scaleFactor;
            float areaWidth = scaledRect.getWidth() / scaleFactor;
            float areaHeight = scaledRect.getHeight() / scaleFactor;
            Area drawArea(areaMarginX,
                          areaMarginY,
                          areaMarginX + areaWidth,
                          areaMarginY + areaHeight);
            
            gl::draw(contentTexture,
                     drawArea,
                     scaledRect);
            
            mScreenTexture.unbind();
            
            if (isJoining)
            {
                gl::lineWidth(5.0f);
                gl::color(ColorAf(1.0f,1.0f,0.0f,std::min<float>(0.8f, alpha)));
            }
            else if (isRemoving)
            {
                gl::lineWidth(5.0f);
                gl::color(ColorAf(1.0f,0.0f,0.0f,std::min<float>(0.8f, alpha)));
            }
            else if (reg.isSelected)
            {
                gl::lineWidth(5.0f);
                gl::color(ColorAf(0.0f,1.0f,0.0f,std::min<float>(0.8f, alpha)));
            }
            else
            {
                gl::lineWidth(2.0f);
                gl::color(1.0f,1.0f,1.0f,std::min<float>(0.8f, alpha));
            }
            gl::drawStrokedRect(scaledRect);
            gl::popMatrices();
        }
        else
        {
            gl::lineWidth(1.0f);
            
            if (isJoining)
            {
                gl::color(ColorAf(1.0f,1.0f,0.0f,alpha));
            }
            else if (isRemoving)
            {
                gl::color(ColorAf(1.0f,0.0f,0.0f,alpha));
            }
            else
            {
                gl::color(ColorAf(0.25f, 0.25f, 0.25f, alpha));
            }
            gl::drawStrokedRect(rA);
        }
    }
}

#pragma mark - Grid Modification

void GridMakerApp::finishAdding(MouseEvent event)
{
    if(mIsMouseValid && mMousePositionEnd != mMousePositionStart)
    {
        ScreenRegion r(mDrawingRect.x1, mDrawingRect.y1, mDrawingRect.x2, mDrawingRect.y2);
        r.isActive = true;
        GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
        curLayout.addRegion(r);
    }
    
    splitScreen();
}

void GridMakerApp::finishRemoving(MouseEvent event)
{
    vector<ScreenRegion> keepRegions;
    GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
    vector<ScreenRegion> regions = curLayout.getRegions();
    int numRegions = regions.size();
    for (int i = 0; i < numRegions; ++i)
    {
        ScreenRegion & reg = regions[i];
        if (!reg.rect.contains(mMousePosition))
        {
            keepRegions.push_back(reg);
        }
    }
    curLayout.setRegions(keepRegions);
    
    splitScreen();
}

void GridMakerApp::finishRegionSelection(MouseEvent event)
{
    int selectedIdx = -1;
    GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
    vector<ScreenRegion> regions = curLayout.getRegions();
    int numRegions = regions.size();
    for (int i = 0; i < numRegions; ++i)
    {
        ScreenRegion & reg = regions[i];
        if (reg.rect.contains(mMousePosition))
        {
            selectedIdx = i;
            reg.isSelected = true;
            mContentName = reg.contentKey;
        }
        else
        {
            reg.isSelected = false;
        }
    }
    // re-set the regions so the selection is updated
    curLayout.setRegions(regions);
    
    if (mSelectedRegionIndex == selectedIdx)
    {
        mSelectedRegionIndex = -1;
    }
    else
    {
        mSelectedRegionIndex = selectedIdx;
    }

    if (mSelectedRegionIndex == -1)
    {
        clearSelection();
    }

    console() << "finish selection. mSelectedRegionIndex: " << mSelectedRegionIndex << endl;
}

void GridMakerApp::finishJoining(MouseEvent event)
{
    console() << "finish joinging. mSelectedRegionIndex: " << mSelectedRegionIndex << endl;
    int selectedIdx = -1;
    GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
    vector<ScreenRegion> regions = curLayout.getRegions();
    int numRegions = regions.size();
    for (int i = 0; i < numRegions; ++i)
    {
        ScreenRegion & reg = regions[i];
        if (reg.rect.contains(mMousePosition))
        {
            selectedIdx = i;
            reg.isSelected = i != mSelectedRegionIndex;
            mContentName = reg.contentKey;
            break;
        }
    }
    // re-set the regions so the selection is updated
    curLayout.setRegions(regions);
    
    if (mSelectedRegionIndex == selectedIdx)
    {
        mSelectedRegionIndex = -1;
    }
    else if (mSelectedRegionIndex != -1)
    {
        // Join them
        ScreenRegion & regA = regions[mSelectedRegionIndex];
        regA.isActive = false;
        ScreenRegion & regB = regions[selectedIdx];
        regB.isActive = false;
        
        int x1 = std::min<int>(regA.rect.x1,regB.rect.x1);
        int x2 = std::max<int>(regA.rect.x2,regB.rect.x2);
        int y1 = std::min<int>(regA.rect.y1,regB.rect.y1);
        int y2 = std::max<int>(regA.rect.y2,regB.rect.y2);
        
        ScreenRegion newReg(x1,y1,x2,y2);
        newReg.isActive = true;
        
        vector<ScreenRegion> keepRegions;
        keepRegions.push_back(newReg);
        
        // Remove any regions that fall within the new region
        for (int i = 0; i < regions.size(); ++i)
        {
            ScreenRegion & reg = regions[i];
            if (!rectsOverlap(reg.rect, newReg.rect))
            {
                keepRegions.push_back(reg);
            }
        }
        curLayout.setRegions(keepRegions);
        
        mSelectedRegionIndex = -1;
        clearSelection();
        
        splitScreen();
    }
    else
    {
        mSelectedRegionIndex = selectedIdx;
    }
    
    if (mSelectedRegionIndex == -1)
    {
        clearSelection();
    }
}

void GridMakerApp::insertNewLayout()
{
    insertLayout(GridLayout());
}

void GridMakerApp::insertLayout(GridLayout newLayout)
{
    clearSelection();

    newLayout.setTransitionDuration(kDefaultTransitionDuration);
    GridLayout & currentLayout = mGridLayouts[mIdxCurrentLayout];
    newLayout.setTimestamp(currentLayout.getTimestamp());
    
    int insertAtPosition = mIdxCurrentLayout + 1;
    mGridLayouts.insert(mGridLayouts.begin() + insertAtPosition, newLayout);
    adjustLayoutTimestamps(insertAtPosition, kDefaultTimestampOffset);

    mIdxPrevLayout = mIdxCurrentLayout;
    mIdxCurrentLayout = insertAtPosition;
    mTransitionAmt = 0.0f;
    calculateTotalDuration();
    newLayoutWasSet();
}

void GridMakerApp::deleteCurrentLayout()
{
    int nextIdx = mIdxCurrentLayout + 1;
    // Ddjust the time of all past the current index
    if (mGridLayouts.size() > nextIdx)
    {
        // We'll need to adjust the time
        GridLayout & layoutRemove = mGridLayouts[mIdxCurrentLayout];
        GridLayout & layoutNext = mGridLayouts[nextIdx];
        long long adjustTime = (layoutNext.getTimestamp() - layoutRemove.getTimestamp()) * -1;
        adjustLayoutTimestamps(nextIdx, adjustTime);
    }
    
    // Delete the current index
    assert(mGridLayouts.size() > 0);
    mGridLayouts.erase(mGridLayouts.begin() + mIdxCurrentLayout);
    
    // Then roll back one if the current index is greater than the grid count
    if (mIdxCurrentLayout >= mGridLayouts.size())
    {
        mIdxCurrentLayout = mGridLayouts.size() - 1;
    }
    
    if (mGridLayouts.size() == 0)
    {
        // Insert a blank one.
        // We should never have zero
        insertLayout(GridLayout());
    }
}

void GridMakerApp::clearCurrentLayout()
{
    GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
    curLayout.setRegions(vector<ScreenRegion>());
}

void GridMakerApp::duplicateCurrentLayout()
{
    GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
    GridLayout dupLayout(curLayout);
    dupLayout.setName(dupLayout.getUniqueID() + ".grid");
    insertLayout(dupLayout);
}

void GridMakerApp::clearSelection()
{
    mContentName = "";
    if (mIdxCurrentLayout != -1)
    {
        GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
        vector<ScreenRegion> regions = curLayout.getRegions();
        for (int i = 0; i < regions.size(); ++i)
        {
            ScreenRegion & reg = regions[i];
            reg.isSelected = false;
        }
        curLayout.setRegions(regions);
    }
}

void GridMakerApp::splitScreen()
{
    mSelectedRegionIndex = -1;
    
    GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
    
    vector<int> xLines;
    xLines.push_back(0);
    xLines.push_back(getWindowWidth() + 1);
    vector<int> yLines;
    yLines.push_back(0);
    yLines.push_back(getWindowHeight() + 1);
    
    vector<ScreenRegion> newRegions;
    vector<ScreenRegion> curRegions = curLayout.getRegions();
    int numRegions = curRegions.size();
    for (int i = 0; i < numRegions; ++i)
    {
        const ScreenRegion & reg = curRegions[i];
        if (reg.isActive)
        {
            newRegions.push_back(reg);
            
            Rectf rect = reg.rect;

            xLines.push_back((int)rect.x1);
            xLines.push_back((int)(rect.x2 + 1));
            
            yLines.push_back((int)rect.y1);
            yLines.push_back((int)(rect.y2 + 1));
        }
    }
    
    // Sort < = >
    // Unique
    sort(xLines.begin(), xLines.end());
    xLines.erase(std::unique(xLines.begin(), xLines.end()), xLines.end());
    
    sort(yLines.begin(), yLines.end());
    yLines.erase(std::unique(yLines.begin(), yLines.end()), yLines.end());

    for (int y = 0; y < yLines.size() - 1; ++y)
    {
        int y1 = yLines[y];
        
        for (int x = 0; x < xLines.size() - 1; ++x)
        {
            int x1 = xLines[x];
            int x2 = xLines[x + 1] - 1;
            int y2 = yLines[y + 1] - 1;
            Rectf testRect(x1, y1, x2, y2);
            if (rectIsValid(testRect, newRegions, false))
            {
                newRegions.push_back(ScreenRegion(x1,y1,x2,y2));
            }
        }
    }

    curLayout.setRegions(newRegions);
}

#pragma mark - Time Editing

// Add or remove time from the playhead
void GridMakerApp::adjustLayoutTimestamps(int startingIndex, long long duration)
{
    int layoutCount = mGridLayouts.size();
    for (int i = startingIndex; i < layoutCount; ++i)
    {
        GridLayout & layout = mGridLayouts[i];
        layout.setTimestamp(layout.getTimestamp() + duration);
    }
}

#pragma mark - Misc

void GridMakerApp::calculateTotalDuration()
{
    int numLayouts = mGridLayouts.size();
    GridLayout & lastLayout = mGridLayouts[numLayouts - 1];
    mTotalDuration = lastLayout.getTimestamp() + lastLayout.getTransitionDuration();
}

CINDER_APP_NATIVE( GridMakerApp, RendererGl )
