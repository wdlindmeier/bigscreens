#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Rect.h"
#include "cinder/gl/Texture.h"
#include "cinder/Utilities.h"
#include "Utilities.hpp"
#include "Helpers.hpp"
#include "cinder/Camera.h"
#include "GridLayout.h"
#include "TankContent.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace bigscreens;

const static float kTimelineDurationMinutes = 4.0f;

// See misc
static double millisecPerPixel();

class TimeLineEditorApp : public AppNative
{
    
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
    
    // App Loop
    void update();
    void draw();
    
    // Misc
    Rectf rectForLayout(int layoutIndex);
    
    // Member vars
    vector<GridLayout> mGridLayouts;
    int mIdxSelectedLayout;
    Vec2f mMousePosition;
};


#pragma mark - Setup

void TimeLineEditorApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(1400, 200);
    settings->setWindowPos(0, 100);
}

void TimeLineEditorApp::setup()
{
    reload();
}

void TimeLineEditorApp::shutdown()
{
    
}

#pragma mark - Load / Save

void TimeLineEditorApp::save()
{
    console() << "Saving\n";
    for (int i = 0; i < mGridLayouts.size(); ++i)
    {
        GridLayout & layout = mGridLayouts[i];
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

void TimeLineEditorApp::loadAllGrids()
{
    fs::path gridPath = getAssetPath(".");
    mGridLayouts = GridLayout::loadAllFromPath(gridPath);
    console() << mGridLayouts.size() << " Layouts found\n";
}

void TimeLineEditorApp::reload()
{
    mIdxSelectedLayout = -1;
    mGridLayouts.clear();
    loadAllGrids();
}

#pragma mark - Mouse

void TimeLineEditorApp::mouseDown( MouseEvent event )
{
    mMousePosition = event.getPos();
    
    int numLayouts = mGridLayouts.size();
    for (int i = 0; i < numLayouts; ++i)
    {
        Rectf layoutRect = rectForLayout(i);
        if (layoutRect.contains(event.getPos()))
        {
            mIdxSelectedLayout = i;
            break;
        }
    }
    if (mIdxSelectedLayout > -1)
    {
        console() << "Selected layout " << mGridLayouts[mIdxSelectedLayout].getName() << endl;
    }
}

void TimeLineEditorApp::mouseUp(MouseEvent event)
{
    mMousePosition = event.getPos();
}

void TimeLineEditorApp::mouseDrag(MouseEvent event)
{
    Vec2f mouseDelta = event.getPos() - mMousePosition;
    mMousePosition = event.getPos();
    
    int timelineDelta = mouseDelta.x * millisecPerPixel();
    if (mIdxSelectedLayout != -1)
    {
        GridLayout & selectedLayout = mGridLayouts[mIdxSelectedLayout];
        string selectedID = selectedLayout.getUniqueID();
        long long timeStamp = selectedLayout.getTimestamp();
        // Update the timestamp
        timeStamp += timelineDelta;
        selectedLayout.setTimestamp(timeStamp);
        
        // Resort in ascending order
        std::sort(mGridLayouts.begin(), mGridLayouts.end(), sortByTimestamp);
        
        // Find the new selected index based on the unique ID
        int numLayouts = mGridLayouts.size();
        for (int i = 0; i < numLayouts; ++i)
        {
            GridLayout &l = mGridLayouts[i];
            if (l.getUniqueID() == selectedID)
            {
                mIdxSelectedLayout = i;
                break;
            }
        }
    }
}

void TimeLineEditorApp::mouseMove(MouseEvent event)
{
    mMousePosition = event.getPos();
}

#pragma mark - Key

void TimeLineEditorApp::keyDown(KeyEvent event)
{
    
}

void TimeLineEditorApp::keyUp(KeyEvent event)
{
    char key = event.getChar();
    if (key == 's')
    {
        save();
    }
    if (key == 'l')
    {
        reload();
    }
}

#pragma mark - App Loop

void TimeLineEditorApp::update()
{
    
}

void TimeLineEditorApp::draw()
{
    gl::clear(ColorA(0,0,0,255));
    gl::enableAlphaBlending();
    gl::lineWidth(1.0f);
    
    // Draw the times
    float winHeight = getWindowHeight();
    float textY = winHeight - 20;
    float xInterval = getWindowWidth() / 4.0f;
    gl::color(ColorAf(0, 1.0f, 1.0f, 1.0f));
    gl::drawString("1:00", Vec2i(xInterval + 5, textY));
    gl::drawLine(Vec2f(xInterval, 0), Vec2f(xInterval, winHeight));
    gl::drawString("2:00", Vec2i(xInterval * 2 + 5, textY));
    gl::drawLine(Vec2f(xInterval * 2, 0), Vec2f(xInterval * 2, winHeight));
    gl::drawString("3:00", Vec2i(xInterval * 3 + 5, textY));
    gl::drawLine(Vec2f(xInterval * 3, 0), Vec2f(xInterval * 3, winHeight));
    
    int numLayouts = mGridLayouts.size();
    for (int i = 0; i < numLayouts; ++i)
    {
        Rectf layoutRect = rectForLayout(i);

        bool isSelected = i == mIdxSelectedLayout;
        if (isSelected)
        {
            gl::color(ColorAf(1.0f,1.0f,0,1.0f));
        }
        else
        {
            gl::color(ColorAf(1.0f,1.0f,1,0.5f));
        }
        gl::drawSolidRect(layoutRect);
        
        if (isSelected)
        {
            gl::color(ColorAf(1.0f,1.0f,0,1.0f));
        }
        else
        {
            gl::color(ColorAf(0.75f,0.75f,0.75,0.5f));
        }
        
        gl::drawStrokedRect(layoutRect);
        gl::drawString(to_string(i), Vec2f(layoutRect.x1 + 2, layoutRect.getHeight() * 0.5),
                       Color::black());
    }
    
    if (mIdxSelectedLayout != -1)
    {
        GridLayout & selectedLayout = mGridLayouts[mIdxSelectedLayout];
        long long msEnter = selectedLayout.getTimestamp();
        long long msFullDuration = kTimelineDurationMinutes * 60 * 1000;
        long long msDuration = msFullDuration - msEnter;
        if (mIdxSelectedLayout < mGridLayouts.size() - 1)
        {
            GridLayout & nextLayout = mGridLayouts[mIdxSelectedLayout+1];
            msDuration = nextLayout.getTimestamp() - msEnter;
        }
        
        gl::drawString(selectedLayout.getName() + " @ " +
                       to_string(msEnter) + " l: " +
                       to_string(msDuration),
                       Vec2i(10, textY));
    }
}

#pragma mark - Misc

static double millisecPerPixel()
{
    return ((double)kTimelineDurationMinutes * 60.0 * 1000.0) / (double)ci::app::getWindowWidth();
}

Rectf TimeLineEditorApp::rectForLayout(int layoutIndex)
{
    double millisecPerPx = millisecPerPixel();
    
    float height = getWindowHeight() - 30;
    
    int numLayouts = mGridLayouts.size();
    GridLayout & layout = mGridLayouts[layoutIndex];
    float x1 = layout.getTimestamp() / millisecPerPx;
    float x2 = getWindowWidth();
    if (layoutIndex < (numLayouts - 1))
    {
        GridLayout & nextLayout = mGridLayouts[layoutIndex+1];
        x2 = nextLayout.getTimestamp() / millisecPerPx;
    }
    return Rectf(x1, 0, x2, height);
}

CINDER_APP_NATIVE( TimeLineEditorApp, RendererGl )