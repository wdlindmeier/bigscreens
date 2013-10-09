#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Rect.h"
#include "cinder/gl/Texture.h"
#include "cinder/Utilities.h"
#include "Helpers.hpp"
#include "cinder/Camera.h"
#include "GridLayout.h"
#include "TankContent.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace bigscreens;

Rectf RectfFromRectInt(RectT<int> rectA)
{
    return Rectf((float)rectA.x1, (float)rectA.y1, (float)rectA.x2, (float)rectA.y2);
}

bool RectsOverlap(Rectf rectA, Rectf rectB)
{
    return rectA.contains(rectB.getUpperLeft()) ||
    rectA.contains(rectB.getUpperRight()) ||
    rectA.contains(rectB.getLowerLeft()) ||
    rectA.contains(rectB.getLowerRight()) ||
    rectB.contains(rectA.getUpperLeft()) ||
    rectB.contains(rectA.getUpperRight()) ||
    rectB.contains(rectA.getLowerLeft()) ||
    rectB.contains(rectA.getLowerRight());
};

bool RectIsValid(const Rectf & rect,
                 const vector<ScreenRegion> & regions,
                 const bool ignoreInactive = true)
{
    bool didOverlap = false;
    for (int i = 0; i < regions.size(); ++i)
    {
        ScreenRegion reg = regions[i];
        if ((reg.isActive || !ignoreInactive) && RectsOverlap(reg.rect, rect))
        {
            didOverlap = true;
            break;
        }
    }
    return !didOverlap;
}

inline Rectf RectFromTwoPos(const Vec2f & posA, const Vec2f & posB)
{
    int x1 = std::min(posA.x, posB.x);
    int x2 = std::max(posA.x, posB.x);
    int y1 = std::min(posA.y, posB.y);
    int y2 = std::max(posA.y, posB.y);
    return Rectf(x1, y1, x2, y2);
}

const int kGridSnapSize = 5;
Vec2i MousePositionSnappedToSize(const Vec2i & pos, const int snapSize)
{
    return pos - Vec2i(pos.x % snapSize, pos.y % snapSize);
}

class GridMakerApp : public AppNative {
  public:
    void prepareSettings(Settings *settings);
	void setup();
	void shutdown();
	void mouseDown( MouseEvent event );
    void mouseUp(MouseEvent event);
    void mouseDrag(MouseEvent event);
    void mouseMove(MouseEvent event);
    void keyDown(KeyEvent event);
    void keyUp(KeyEvent event);
    void clearSelection();
	void update();
	void draw();
    void drawLayout(const GridLayout & layout,
                    const float scalarAmount,
                    const GridLayout & compareTolayout);
    void splitScreen();
    void finishJoining(MouseEvent event);
    void finishRemoving(MouseEvent event);
    void finishAdding(MouseEvent event);
    void loadAllGrids();
    void save();

    vector<GridLayout> mGridLayouts;
    int mIdxCurrentLayout;
    int mIdxPrevLayout;
    int mSelectedRegionIndex;
    bool mIsAdding;
    bool mIsRemoving;
    bool mIsJoining;
    bool mIsMouseValid;
    Vec2i mMousePositionStart;
    Vec2i mMousePositionEnd;
    Vec2i mMousePosition;
    Rectf mDrawingRect;
    float mTransitionAmt;
    
    ci::gl::Texture mScreenTexture;
    TankContent mContent;
};

void GridMakerApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(960, 270);
}

void GridMakerApp::setup()
{
    mIsAdding = false;
    mIsRemoving = false;
    mIsMouseValid = true;
    mIsJoining = false;
    mSelectedRegionIndex = -1;
    mIdxCurrentLayout = -1;
    mIdxPrevLayout = -1;
    mTransitionAmt = 1.0;
    loadAllGrids();
    if (mIdxCurrentLayout > -1)
    {
        splitScreen();
    }
    else
    {
        console() << "Didn't find any serialized grids." << endl;
        // Add an empty layout
        mGridLayouts.push_back(GridLayout());
        mIdxCurrentLayout = 0;
    }
    
    mScreenTexture = loadImage(app::loadResource("screen.png"));
    mContent.load("T72.obj");
}

void GridMakerApp::shutdown()
{
    // Save current state
    save();
}

void GridMakerApp::save()
{
    for (int i = 0; i < mGridLayouts.size(); ++i)
    {
        mIdxCurrentLayout = i;
        GridLayout & layout = mGridLayouts[mIdxCurrentLayout];
        if (layout.getRegions().size() > 0)
        {
            layout.serialize();
        }
        else
        {
            layout.remove();
        }
    }
}

void GridMakerApp::loadAllGrids()
{
    fs::path gridPath = getAssetPath(".");
    mGridLayouts = GridLayout::loadAllFromPath(gridPath);
    if (mGridLayouts.size() > 0)
    {
        mIdxCurrentLayout = 0;
    }
}

void GridMakerApp::mouseDown( MouseEvent event )
{
    mMousePositionStart = MousePositionSnappedToSize(event.getPos(), kGridSnapSize);
    mDrawingRect = Rectf(mMousePosition.x, mMousePosition.y,
                              mMousePosition.x, mMousePosition.y);
}

void GridMakerApp::mouseDrag(MouseEvent event)
{
    mMousePosition = MousePositionSnappedToSize(event.getPos(), kGridSnapSize);
    
    if (mIsAdding)
    {
        mDrawingRect = RectFromTwoPos(mMousePosition, mMousePositionStart);
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
    mIsMouseValid = RectIsValid(mDrawingRect, curLayout.getRegions());
    
    if (mIsMouseValid)
    {
        mMousePositionEnd = mMousePosition;
    }
}

void GridMakerApp::mouseMove(MouseEvent event)
{
    mMousePosition = MousePositionSnappedToSize(event.getPos(), kGridSnapSize);
    // +1 to avoid snap-to-grid collisions
    mDrawingRect = Rectf(mMousePosition.x + 1, mMousePosition.y + 1,
                              mMousePosition.x + 1, mMousePosition.y + 1);
    GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
    mIsMouseValid = RectIsValid(mDrawingRect, curLayout.getRegions());
}

void GridMakerApp::mouseUp(MouseEvent event)
{
    if (mIsAdding)
    {
        finishAdding(event);
    }

    if (mIsRemoving)
    {
        finishRemoving(event);
    }
    
    if(mIsJoining)
    {
        finishJoining(event);
    }
}

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

void GridMakerApp::finishJoining(MouseEvent event)
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
            reg.isSelected = i != mSelectedRegionIndex;
            break;
        }
    }
    // re-set the regions so the selection is updated
    curLayout.setRegions(regions);
    
    if (mSelectedRegionIndex == selectedIdx)
    {
        mSelectedRegionIndex = -1;
        clearSelection();
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
            if (!RectsOverlap(reg.rect, newReg.rect))
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
}

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
    if (!isJoining && mIsJoining)
    {
        clearSelection();
    }
    mIsJoining = isJoining;
    
    mIsRemoving = event.isAltDown();
    
    GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
    
    char key = event.getChar();
    if (key == 's')
    {
        splitScreen();
    }
    else if (key == ' ')
    {
        curLayout.setRegions(vector<ScreenRegion>());
    }
    else if (key == 'w')
    {
        save();
    }
    else if (key == 'n')
    {
        mGridLayouts.push_back(GridLayout());
        mIdxPrevLayout = mIdxCurrentLayout;
        mIdxCurrentLayout = mGridLayouts.size() - 1;
        mTransitionAmt = 0.0f;
    }
    else if (key == 'd')
    {
        GridLayout dupLayout(curLayout);
        dupLayout.setName("");
        mGridLayouts.push_back(dupLayout);
        mIdxPrevLayout = mIdxCurrentLayout;
        mIdxCurrentLayout = mGridLayouts.size() - 1;
        mTransitionAmt = 0.0f;
    }
    else if (event.getCode() == KeyEvent::KEY_RIGHT)
    {
        mIdxPrevLayout = mIdxCurrentLayout;
        mIdxCurrentLayout = (mIdxCurrentLayout + 1) % mGridLayouts.size();
        mTransitionAmt = 0.0f;
        splitScreen();
    }
    else if (event.getCode() == KeyEvent::KEY_LEFT)
    {
        mIdxPrevLayout = mIdxCurrentLayout;
        mIdxCurrentLayout = mIdxCurrentLayout - 1;
        if (mIdxCurrentLayout < 0)
        {
            mIdxCurrentLayout = mGridLayouts.size() - 1;
        }
        mTransitionAmt = 0.0f;
        splitScreen();
    }
}

void GridMakerApp::clearSelection()
{
    GridLayout & curLayout = mGridLayouts[mIdxCurrentLayout];
    vector<ScreenRegion> regions = curLayout.getRegions();
    for (int i = 0; i < regions.size(); ++i)
    {
        ScreenRegion & reg = regions[i];
        reg.isSelected = false;
    }
}

const static float kTransitionStep = 0.05;

void GridMakerApp::update()
{
    if (mTransitionAmt < 1.0)
    {
        mTransitionAmt = std::min<float>(mTransitionAmt + kTransitionStep, 1.0f);
    }
    
    mContent.update();
}

void GridMakerApp::draw()
{
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

        prevLayout.render(1.0 - std::min(mTransitionAmt / 0.5f, 1.0f),
                          curLayout,
                          mMousePosition,
                          mode,
                          mContent);
        
        curLayout.render(std::max((mTransitionAmt - 0.5f) / 0.5f, 0.0f),
                         prevLayout,
                         mMousePosition,
                         mode,
                         mContent);

    }
    else
    {
        curLayout.render(mTransitionAmt,
                         GridLayout(),
                         mMousePosition,
                         mode,
                         mContent);
    }

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
            if (RectIsValid(testRect, newRegions, false))
            {
                newRegions.push_back(ScreenRegion(x1,y1,x2,y2));
            }
        }
    }

    curLayout.setRegions(newRegions);

}

CINDER_APP_NATIVE( GridMakerApp, RendererGl )
