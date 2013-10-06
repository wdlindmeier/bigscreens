#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"
#include "Helpers.hpp"
#include <boost/iterator/filter_iterator.hpp>
#include <fstream>

using namespace ci;
using namespace ci::app;
using namespace std;

struct ScreenRegion
{
    Rectf rect;
    bool isActive;
    ColorAf color;

    ScreenRegion(float x1, float y1, float x2, float y2) :
    rect(x1,y1,x2,y2),
    isActive(false),
    color(Rand::randFloat(),Rand::randFloat(),Rand::randFloat(),1.0f)
    {
    };
};

struct Layout
{
    vector<ScreenRegion> regions;
    string name;
    
    Layout() : name(""){};
};

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
	void setup();
	void shutdown();
	void mouseDown( MouseEvent event );
    void mouseUp(MouseEvent event);
    void mouseDrag(MouseEvent event);
    void mouseMove(MouseEvent event);
    void keyDown(KeyEvent event);
    void keyUp(KeyEvent event);
	void update();
	void draw();
    void splitScreen();
    void finishJoining(MouseEvent event);
    void finishRemoving(MouseEvent event);
    void finishAdding(MouseEvent event);
    void serialize();
    void loadAllGrids();
    Layout load(fs::path readPath);

    vector<Layout> mGridLayouts;
    int mIdxCurrentLayout;
    int mSelectedRegionIndex;
    bool mIsAdding;
    bool mIsRemoving;
    bool mIsJoining;
    bool mIsMouseValid;
    Vec2i mMousePositionStart;
    Vec2i mMousePositionEnd;
    Vec2i mMousePosition;
    Rectf mDrawingRect;
};

void GridMakerApp::setup()
{
    mIsAdding = false;
    mIsRemoving = false;
    mIsMouseValid = true;
    mIsJoining = false;
    mSelectedRegionIndex = -1;
    mIdxCurrentLayout = -1;
    loadAllGrids();
    if (mIdxCurrentLayout > -1)
    {
        splitScreen();
    }
    else
    {
        console() << "Didn't find any serialized grids." << endl;
        // Add an empty layout
        mGridLayouts.push_back(Layout());
        mIdxCurrentLayout = 0;
    }
}

void GridMakerApp::shutdown()
{
    // Save current state
    for (int i = 0; i < mGridLayouts.size(); ++i)
    {
        mIdxCurrentLayout = i;
        Layout & layout = mGridLayouts[mIdxCurrentLayout];
        if (layout.regions.size() > 0)
        {
            serialize();
        }
        else
        {
            // Just delete them if they're empty
            string filename = layout.name;
            if (filename != "")
            {
                fs::path gridPath = getAppPath() / ".." / filename;
                fs::remove(gridPath);
            }
        }
    }
}

void GridMakerApp::serialize()
{
    Layout & layout = mGridLayouts[mIdxCurrentLayout];
    
    string filename = layout.name;
    if (filename == "")
    {
        time_t t;
        long timestamp = time(&t);
        filename = to_string(timestamp) + ".grid";
    }
    fs::path gridPath = getAppPath() / ".." / filename;
    
    std::ofstream oStream( gridPath.string() );

    vector<string> output;
    for (int i = 0; i < layout.regions.size(); ++i)
    {
        ScreenRegion & reg = layout.regions[i];
        if (reg.isActive)
        {
            oStream << to_string((int)reg.rect.x1) << ",";
            oStream << to_string((int)reg.rect.y1) << ",";
            oStream << to_string((int)reg.rect.x2) << ",";
            oStream << to_string((int)reg.rect.y2) << ",";
            oStream << "\n";
        }
    }
    oStream.close();
}

void GridMakerApp::loadAllGrids()
{
    fs::path gridPath = getAppPath() / "..";
    fs::directory_iterator dir_first(gridPath), dir_last;
    
    // Filter on the .grid filetype
    auto pred = [](const fs::directory_entry& p)
    {
        string filename = string(p.path().filename().c_str());
        std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
        return fs::is_regular_file(p) && filename.find( ".grid" ) != -1;
    };
    
    std::vector<fs::path> gridFiles;
    std::copy(make_filter_iterator(pred, dir_first, dir_last),
              make_filter_iterator(pred, dir_last, dir_last),
              std::back_inserter(gridFiles));
    
    mGridLayouts.clear();
    for (int i = 0; i < gridFiles.size(); ++i)
    {
        Layout gridLayout = load(gridFiles[i]);
        if (gridLayout.regions.size() > 0)
        {
            mGridLayouts.push_back(gridLayout);
        }
    }
    if (mGridLayouts.size() > 0)
    {
        mIdxCurrentLayout = 0;
    }
}

Layout GridMakerApp::load(fs::path readPath)
{
    Layout layout;
    if (fs::exists(readPath))
    {
        fstream inFile(readPath.string());
        string line;
        while (getline (inFile, line))
        {
            vector<string> tokens = ci::split(line, ",");
            int x1 = stoi(tokens[0]);
            int y1 = stoi(tokens[1]);
            int x2 = stoi(tokens[2]);
            int y2 = stoi(tokens[3]);
            ScreenRegion reg(x1,y1,x2,y2);
            reg.isActive = true;
            layout.regions.push_back(reg);
        }
        
        layout.name = readPath.filename().string();
    }
    else
    {
        console() << "ERROR: File doesn't exist at path " << readPath << endl;
    }
    return layout;
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

    Layout & curLayout = mGridLayouts[mIdxCurrentLayout];
    mIsMouseValid = RectIsValid(mDrawingRect, curLayout.regions);
    
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
    Layout & curLayout = mGridLayouts[mIdxCurrentLayout];
    mIsMouseValid = RectIsValid(mDrawingRect, curLayout.regions);
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
        Layout & curLayout = mGridLayouts[mIdxCurrentLayout];
        curLayout.regions.push_back(r);
    }
    
    splitScreen();
}

void GridMakerApp::finishRemoving(MouseEvent event)
{
    vector<ScreenRegion> keepRegions;
    Layout & curLayout = mGridLayouts[mIdxCurrentLayout];
    for (int i = 0; i < curLayout.regions.size(); ++i)
    {
        ScreenRegion & reg = curLayout.regions[i];
        if (!reg.rect.contains(mMousePosition))
        {
            keepRegions.push_back(reg);
        }
    }
    curLayout.regions = keepRegions;
    
    splitScreen();
}

void GridMakerApp::finishJoining(MouseEvent event)
{
    int selectedIdx = -1;
    Layout & curLayout = mGridLayouts[mIdxCurrentLayout];
    for (int i = 0; i < curLayout.regions.size(); ++i)
    {
        ScreenRegion reg = curLayout.regions[i];
        if (reg.rect.contains(mMousePosition))
        {
            selectedIdx = i;
            break;
        }
    }
    
    if (mSelectedRegionIndex == selectedIdx)
    {
        mSelectedRegionIndex = -1;
    }
    else if (mSelectedRegionIndex != -1)
    {
        // Join them
        ScreenRegion & regA = curLayout.regions[mSelectedRegionIndex];
        regA.isActive = false;
        ScreenRegion & regB = curLayout.regions[selectedIdx];
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
        for (int i = 0; i < curLayout.regions.size(); ++i)
        {
            ScreenRegion & reg = curLayout.regions[i];
            if (!RectsOverlap(reg.rect, newReg.rect))
            {
                keepRegions.push_back(reg);
            }
        }
        curLayout.regions = keepRegions;

        mSelectedRegionIndex = -1;
        
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
    mIsJoining = event.isControlDown();
    mIsRemoving = event.isAltDown();
    
    Layout & curLayout = mGridLayouts[mIdxCurrentLayout];
    
    char key = event.getChar();
    if (key == 's')
    {
        splitScreen();
    }
    else if (key == ' ')
    {
        curLayout.regions.clear();
    }
    else if (key == 'w')
    {
        serialize();
    }
    else if (key == 'n')
    {
        mGridLayouts.push_back(Layout());
        mIdxCurrentLayout = mGridLayouts.size() - 1;
    }
    else if (event.getCode() == KeyEvent::KEY_RIGHT)
    {
        mIdxCurrentLayout = (mIdxCurrentLayout + 1) % mGridLayouts.size();
        // TODO; Transition
        splitScreen();
    }
    else if (event.getCode() == KeyEvent::KEY_LEFT)
    {
        mIdxCurrentLayout = mIdxCurrentLayout - 1;
        if (mIdxCurrentLayout < 0)
        {
            mIdxCurrentLayout = mGridLayouts.size() - 1;
        }
        // TODO; Transition
        splitScreen();
    }
}

void GridMakerApp::update()
{
}

void GridMakerApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::lineWidth(1);

    Layout & curLayout = mGridLayouts[mIdxCurrentLayout];
    
    // Draw the rects
    for (int i = 0; i < curLayout.regions.size(); ++i)
    {
        ScreenRegion reg = curLayout.regions[i];
        bool isHovered = reg.rect.contains(mMousePosition);
        bool isJoining = (mSelectedRegionIndex == i) || (mIsJoining && isHovered);
        bool isRemoving = (mIsRemoving && isHovered);
        
        if(reg.isActive)
        {
            if (isJoining)
            {
                gl::color(ColorAf(1.0f,1.0f,0.0f,1.0f));
            }
            else if (isRemoving)
            {
                gl::color(ColorAf(1.0f,0.0f,0.0f,1.0f));
            }
            else
            {
                gl::color(reg.color);
            }
            gl::drawSolidRect(reg.rect);
        }
        else
        {
            if (isJoining)
            {
                gl::color(ColorAf(1.0f,1.0f,0.0f,1.0f));
            }
            else if (isRemoving)
            {
                gl::color(ColorAf(1.0f,0.0f,0.0f,1.0f));
            }
            else
            {
                gl::color(0.5f, 0.5f, 0.5f);
            }
            gl::drawStrokedRect(reg.rect);
        }
    }

    float height = getWindowHeight();
    float width = getWindowWidth();
    
    // Draw the active rect
    if (mIsAdding)
    {
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
    }
}

void GridMakerApp::splitScreen()
{
    mSelectedRegionIndex = -1;
    
    Layout & curLayout = mGridLayouts[mIdxCurrentLayout];
    
    vector<int> xLines;
    xLines.push_back(0);
    xLines.push_back(getWindowWidth() + 1);
    vector<int> yLines;
    yLines.push_back(0);
    yLines.push_back(getWindowHeight() + 1);
    
    vector<ScreenRegion> newRegions;
    
    for (int i = 0; i < curLayout.regions.size(); ++i)
    {
        const ScreenRegion & reg = curLayout.regions[i];
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
            
            int incX = 1;
            int incY = 1;
            bool didFit = false;
            
            for (int i = 0; i < 2; ++i)
            {
                bool didHitWall = false;
                while(!didHitWall)
                {
                    int x2 = xLines[x + incX] - 1;
                    int y2 = yLines[y + incY] - 1;
                    Rectf testRect(x1,y1,x2,y2);
                    if(RectIsValid(testRect, newRegions, false))
                    {
                        didFit = true;
                        // This will try to consume more empty plots, but I think
                        // it's better to just do it manually.
                        /*
                        if (i == 0)
                        {
                            if ((x + incX) >= xLines.size() - 1)
                            {
                                didHitWall = true;
                            }
                            else
                            {
                                ++incX;
                            }
                        }
                        else
                        {
                            if ((y + incY) >= yLines.size() - 1)
                            {
                                didHitWall = true;
                            }
                            else
                            {
                                ++incY;
                            }
                        }*/
                    }
                    /*
                    else
                    {
                        if (i == 0)
                        {
                            --incX;
                        }
                        else
                        {
                            --incY;
                        }
                        didHitWall = true;
                    }
                    */
                    break;
                }
            }
            
            if (didFit)
            {
                int x2 = xLines[x + incX] - 1;
                int y2 = yLines[y + incY] - 1;
                newRegions.push_back(ScreenRegion(x1,y1,x2,y2));
            }
        }
    }

    curLayout.regions = newRegions;
    
}

CINDER_APP_NATIVE( GridMakerApp, RendererGl )
