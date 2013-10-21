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
#include "Slider.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace bigscreens;

const static float kTimelineDurationMinutes = 4.0f;
const static long long kMsFullDuration = kTimelineDurationMinutes * 60 * 1000;
const static long kRangeTransition = 20000; // 20 seconds
const static float kPositionSliderMagnitude = 100.0f; // 10 ms

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
    
    // Editing
    void adjustSliderPositionForMouseMove();
    void adjustSelectedLayoutTimestamp(long long timeDelta);
    
    // Key
    void keyDown(KeyEvent event);
    void keyUp(KeyEvent event);
    
    // App Loop
    void update();
    void draw();
    void renderSliders();
    
    // Misc
    Rectf rectForLayout(int layoutIndex);
    void selectLayout(int layoutIndex);
    void updateSlidersForSelectedLayout();
    
    // Member vars
    vector<GridLayout> mGridLayouts;
    int mIdxSelectedLayout;
    Vec2f mMousePosition;
    bool mIsShiftDown;
    
    Slider mSliderPosition;
    Slider mSliderTransition;
    
};


#pragma mark - Setup

void TimeLineEditorApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(1400, 200);
    settings->setWindowPos(0, 100);
}

void TimeLineEditorApp::setup()
{
    mMousePosition = Vec2f::zero();
    mIsShiftDown = false;
    float sliderWidth = (getWindowWidth() * 0.5) - 300.f;
    float sliderOffsetX = 120.0f;
    float sliderOffsetY = 10.0f;
    float sliderHeight = 20.0f;
    float xDuration = sliderOffsetX;
    mSliderPosition = Slider(Rectf(xDuration,
                                   sliderOffsetY,
                                   xDuration + sliderWidth,
                                   sliderOffsetY + sliderHeight));
    mSliderPosition.setValue(0.5f);
    float xTransition = (getWindowWidth() * 0.5f) + sliderOffsetX;
    mSliderTransition = Slider(Rectf(xTransition,
                                     sliderOffsetY,
                                     xTransition + sliderWidth,
                                     sliderOffsetY + sliderHeight));
    reload();
}

void TimeLineEditorApp::shutdown()
{
    
}

#pragma mark - Load / Save

void TimeLineEditorApp::save()
{
    console() << "Saving\n";

    fs::path gridPath = SharedGridPath();

    for (int i = 0; i < mGridLayouts.size(); ++i)
    {
        GridLayout & layout = mGridLayouts[i];
        if (layout.getRegions().size() > 0)
        {
            layout.serialize(gridPath);
        }
        else
        {
            layout.remove();
        }
    }
}

void TimeLineEditorApp::loadAllGrids()
{
    fs::path gridPath = SharedGridPath();
    mGridLayouts = GridLayout::loadAllFromPath(gridPath);
    console() << mGridLayouts.size() << " Layouts found\n";
}

void TimeLineEditorApp::reload()
{
    selectLayout(-1);
    mGridLayouts.clear();
    loadAllGrids();
}

#pragma mark - Mouse

void TimeLineEditorApp::mouseDown( MouseEvent event )
{
    mMousePosition = event.getPos();

    if (mIdxSelectedLayout != -1)
    {
        mSliderPosition.setIsActive(mSliderPosition.contains(mMousePosition));
        mSliderTransition.setIsActive(mSliderTransition.contains(mMousePosition));
    }

    // Only select if we didn't click into a slider.
    // If we did, maintain the previous selection.
    if (!mSliderPosition.getIsActive() &&
        !mSliderTransition.getIsActive())
    {
        selectLayout(-1);
        
        int numLayouts = mGridLayouts.size();
        for (int i = 0; i < numLayouts; ++i)
        {
            Rectf layoutRect = rectForLayout(i);
            if (layoutRect.contains(event.getPos()))
            {
                selectLayout(i);
                break;
            }
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
    mSliderPosition.setIsActive(false);
    mSliderPosition.setValue(0.5f);
    mSliderTransition.setIsActive(false);
}

void TimeLineEditorApp::mouseDrag(MouseEvent event)
{
    Vec2f mouseDelta = event.getPos() - mMousePosition;
    mMousePosition = event.getPos();

    if (mSliderTransition.getIsActive())
    {
        mSliderTransition.update(mMousePosition);
        assert(mIdxSelectedLayout > -1);
        GridLayout & selectedLayout = mGridLayouts[mIdxSelectedLayout];
        long long transitionDuration = mSliderTransition.getValue() * kRangeTransition;
        selectedLayout.setTransitionDuration(transitionDuration);
    }
    else if (mIdxSelectedLayout > -1 &&
             !mSliderPosition.getIsActive())
    {
        long long timelineDelta = mouseDelta.x * millisecPerPixel();
        adjustSelectedLayoutTimestamp(timelineDelta);
    }
}

void TimeLineEditorApp::mouseMove(MouseEvent event)
{
    mMousePosition = event.getPos();
}

#pragma mark - Editing

void TimeLineEditorApp::adjustSliderPositionForMouseMove()
{
    if (mSliderPosition.getIsActive())
    {
        mSliderPosition.update(mMousePosition);
        assert(mIdxSelectedLayout > -1);
        long long timeDelta = (mSliderPosition.getValue() - 0.5f) * kPositionSliderMagnitude;
        adjustSelectedLayoutTimestamp(timeDelta);
    }
}

void TimeLineEditorApp::adjustSelectedLayoutTimestamp(long long timeDelta)
{
    if (mIsShiftDown)
    {
        // Push all of the layouts
        int numLayouts = mGridLayouts.size();
        for (int i = mIdxSelectedLayout; i < numLayouts; ++i)
        {
            GridLayout & layout = mGridLayouts[i];
            long long timeStamp = layout.getTimestamp();
            
            // Update the timestamp
            timeStamp = std::max<long long>(0, std::min<long long>(timeStamp + timeDelta, kMsFullDuration));
            layout.setTimestamp(timeStamp);
        }
    }
    else
    {
        // Drag the selected layout
        GridLayout & selectedLayout = mGridLayouts[mIdxSelectedLayout];
        string selectedID = selectedLayout.getUniqueID();
        long long timeStamp = selectedLayout.getTimestamp();
        // Update the timestamp
        timeStamp = std::max<long long>(0, std::min<long long>(timeStamp + timeDelta, kMsFullDuration));
        selectedLayout.setTimestamp(timeStamp);
        
        // Resort in ascending order
        std::sort(mGridLayouts.begin(), mGridLayouts.end(), sortGridLayoutsByTimestamp);
        
        // Find the new selected index based on the unique ID
        int numLayouts = mGridLayouts.size();
        for (int i = 0; i < numLayouts; ++i)
        {
            GridLayout &l = mGridLayouts[i];
            if (l.getUniqueID() == selectedID)
            {
                //mIdxSelectedLayout = i;
                selectLayout(i);
                break;
            }
        }
    }
    updateSlidersForSelectedLayout();
}

#pragma mark - Key

void TimeLineEditorApp::keyDown(KeyEvent event)
{
    mIsShiftDown = event.isShiftDown();
}

void TimeLineEditorApp::keyUp(KeyEvent event)
{
    mIsShiftDown = event.isShiftDown();

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
    adjustSliderPositionForMouseMove();
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

        // Draw the body
        bool isSelected = i == mIdxSelectedLayout;
        if (isSelected)
        {
            gl::color(ColorAf(1.0f,1.0f,0,1.0f));
        }
        else if (mIsShiftDown && mIdxSelectedLayout > -1 && i > mIdxSelectedLayout)
        {
            // These will also be moved, so color them
            gl::color(ColorAf(0.65f,0.8f,0,1.0f));
        }
        else
        {
            gl::color(ColorAf(1.0f,1.0f,1,0.5f));
        }
        gl::drawSolidRect(layoutRect);
        
        // Draw the stroke
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
        
        // Draw the transition time
        // Magenta
        GridLayout & layout = mGridLayouts[i];
        float transitionWidth = layout.getTransitionDuration() / millisecPerPixel();
        Rectf transitionRect = layoutRect;
        transitionRect.x2 = transitionRect.x1 + transitionWidth;
        gl::color(ColorAf(1.0f,0.0f,1.0f,0.2f));
        gl::drawSolidRect(transitionRect);
        
    }
    
    if (mIdxSelectedLayout != -1)
    {
        GridLayout & selectedLayout = mGridLayouts[mIdxSelectedLayout];
        long long msEnter = selectedLayout.getTimestamp();
        long long msDuration = kMsFullDuration - msEnter;
        if (mIdxSelectedLayout < mGridLayouts.size() - 1)
        {
            GridLayout & nextLayout = mGridLayouts[mIdxSelectedLayout+1];
            msDuration = nextLayout.getTimestamp() - msEnter;
        }
        
        float seconds = msDuration / 1000.0;
        int minutes = (int)seconds / 60;
        if (minutes > 0)
        {
            seconds -= minutes * 60;
        }
        
        string outp = "\"" + selectedLayout.getName() + "\" - Duration: ";
        if (minutes > 0)
        {
            outp += to_string(minutes) + ":";
        }
        
        char buff[100];
        sprintf(buff, "%2.1f", seconds);
        std::string buffAsStdStr = buff;
        outp += buffAsStdStr;
        
        if (minutes == 0)
        {
            outp += " sec";
        }
        
        gl::drawString(outp, Vec2i(10, textY));
    }
    
    renderSliders();
}

void TimeLineEditorApp::renderSliders()
{
    float labelY = 18;
    mSliderPosition.render(mIdxSelectedLayout > -1);
    gl::drawString("Timestamp (ms): ", Vec2f(10, labelY));
    long long positionMS = 0;
    if (mIdxSelectedLayout != -1)
    {
        positionMS  = mGridLayouts[mIdxSelectedLayout].getTimestamp();
    }
    gl::drawString(to_string((long)positionMS),
                   Vec2f((getWindowWidth() * 0.5) - 150,
                         labelY));

    mSliderTransition.render(mIdxSelectedLayout > -1);
    gl::drawString("Transition (ms): ", Vec2f((getWindowWidth() * 0.5f) + 10, labelY));
    gl::drawString(to_string((long)(mSliderTransition.getValue() * kRangeTransition)),
                   Vec2f(getWindowWidth() - 150,
                         labelY));
}

#pragma mark - Misc

static double millisecPerPixel()
{
    return ((double)kTimelineDurationMinutes * 60.0 * 1000.0) / (double)ci::app::getWindowWidth();
}

Rectf TimeLineEditorApp::rectForLayout(int layoutIndex)
{
    double millisecPerPx = millisecPerPixel();
    
    float vertMarginTop = 40;
    float vertMarginBottom = 30;
    float height = getWindowHeight() - (vertMarginBottom + vertMarginTop);
    
    int numLayouts = mGridLayouts.size();
    GridLayout & layout = mGridLayouts[layoutIndex];
    float x1 = layout.getTimestamp() / millisecPerPx;
    float x2 = getWindowWidth();
    if (layoutIndex < (numLayouts - 1))
    {
        GridLayout & nextLayout = mGridLayouts[layoutIndex+1];
        x2 = nextLayout.getTimestamp() / millisecPerPx;
    }
    return Rectf(x1, vertMarginTop, x2, vertMarginTop + height);
}

void TimeLineEditorApp::selectLayout(int layoutIndex)
{
    mIdxSelectedLayout = layoutIndex;
    updateSlidersForSelectedLayout();
}

void TimeLineEditorApp::updateSlidersForSelectedLayout()
{
    if (mIdxSelectedLayout > -1)
    {
        GridLayout & layout = mGridLayouts[mIdxSelectedLayout];
        /*
        mSliderPosition.setValue((long double)layout.getTimestamp() /
                                 (float)kRangePosition);
        */
        mSliderTransition.setValue((long double)layout.getTransitionDuration() /
                                   (float)kRangeTransition);
    }
    else
    {
        //mSliderPosition.setValue(0);
        mSliderTransition.setValue(0);
    }
}

CINDER_APP_NATIVE( TimeLineEditorApp, RendererGl )