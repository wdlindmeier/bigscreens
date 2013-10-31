//
//  GridLayoutTimeline.cpp
//  GridMaker
//
//  Created by William Lindmeier on 10/17/13.
//
//

#include "GridLayoutTimeline.h"
#include "Utilities.hpp"

using namespace ci;
using namespace std;

namespace bigscreens
{

    GridLayoutTimeline::GridLayoutTimeline( const ci::fs::path & filePath, float scale) :
    mLoadPath(filePath),
    mScale(scale),
    mGridLayouts(),
    mIdxCurrentLayout(-1),
    mIdxPrevLayout(-1),
    mTransitionAmt(0),
    mStartTime(0),
    mPlayheadTime(0),
    mLastFrameTime(0),
    mTotalDuration(0),
    mPlaybackSpeed(1.0f),
    mIsPlaying(false)
    {
        reload();
    }

#pragma mark - Layout Management
    
    void GridLayoutTimeline::loadAllGrids()
    {
        ci::app::console() << "Reloading grids from " << mLoadPath << endl;
        mGridLayouts = GridLayout::loadAllFromPath(mLoadPath, mScale);
        int numLayouts = mGridLayouts.size();
        if (numLayouts > 0)
        {
            mIdxCurrentLayout = 0;
        }
        assignTimelineIDs();
    }

    // This looks at each screen region and assigns it a contiguous timeline ID so
    // we can track the content across layouts.
    void GridLayoutTimeline::assignTimelineIDs()
    {
        int regionID = 0;
        for (int i = 0; i < mGridLayouts.size(); ++i)
        {
            GridLayout & layout = mGridLayouts[i];
            vector<ScreenRegion> newRegions;
            vector<ScreenRegion> prevRegions;
            if (i > 0)
            {
                prevRegions = mGridLayouts[i-1].getRegions();
            }
            for (ScreenRegion & region : layout.getRegions() )
            {
                // Check if the layout is zero or if this region
                // existed in the previous layout.
                // If so, use the previous ID.
                int newRegionID = -1;
                if (i != 0)
                {
                    // Compare against prev regions
                    for (ScreenRegion & prevReg : prevRegions)
                    {
                        if (rectCompare(prevReg.rect, region.rect))
                        {
                            newRegionID = prevReg.timelineID;
                            break;
                        }
                    }
                }
                if (newRegionID == -1)
                {
                    newRegionID = ++regionID;
                }
                region.timelineID = newRegionID;
                newRegions.push_back(region);
            }
            // re-set
            layout.setRegions(newRegions);
        }
    }
    
    void GridLayoutTimeline::reload()
    {
        mGridLayouts.clear();

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
        
        if (mIdxCurrentLayout == -1)
        {
            ci::app::console() << "Didn't find any serialized grids." << endl;
            // Add an empty layout
            GridLayout newLayout;
            newLayout.setTimestamp(0);
            newLayout.setTransitionDuration(kDefaultTransitionDuration);
            mGridLayouts.push_back(newLayout);
            
            mIdxCurrentLayout = 0;
        }
        
        newLayoutWasSet();
    }
    
    void GridLayoutTimeline::newLayoutWasSet()
    {
        mTransitionAmt = mIsPlaying ? 0.0f : 1.0f;
        
        // Always reset the playhead so if we fwd/bck while playing it
        // continues from the current frame.
        GridLayout & layout = mGridLayouts[mIdxCurrentLayout];
        mPlayheadTime = layout.getTimestamp();
    }

#pragma mark - Playback Controls
    
    void GridLayoutTimeline::stepToNextLayout()
    {
        mIdxPrevLayout = mIdxCurrentLayout;
        mIdxCurrentLayout = (mIdxCurrentLayout + 1) % mGridLayouts.size();
        newLayoutWasSet();
    }
    
    void GridLayoutTimeline::stepToPreviousLayout()
    {
        mIdxPrevLayout = mIdxCurrentLayout;
        mIdxCurrentLayout = mIdxCurrentLayout - 1;
        if (mIdxCurrentLayout < 0)
        {
            mIdxCurrentLayout = mGridLayouts.size() - 1;
        }
        newLayoutWasSet();
    }
    
    void GridLayoutTimeline::restart()
    {
        mStartTime = 0;
        mIdxCurrentLayout = 0;
        mTransitionAmt = 1.0f;
        mPlayheadTime = 0;
    }
    
    void GridLayoutTimeline::play()
    {
        mLastFrameTime = getMilliCount();
        mIsPlaying = true;
    }
    
    void GridLayoutTimeline::pause()
    {
        mIsPlaying = false;
    }
    
    bool GridLayoutTimeline::isPlaying()
    {
        return mIsPlaying;
    }
    
#pragma mark - App Loop
    
    void GridLayoutTimeline::update()
    {
        if (mIsPlaying)
        {
            long long timestamp = getMilliCount();
            
            long long timeDelta = (timestamp - mLastFrameTime) * mPlaybackSpeed;
            mPlayheadTime += timeDelta;
            
            mLastFrameTime = timestamp;
            
            int nextID = mIdxCurrentLayout + 1;
            
            // Never loop
            if (nextID < mGridLayouts.size())
            {
                GridLayout & nextLayout = mGridLayouts[nextID];
                
                long long startTimeNextLayout = nextLayout.getTimestamp();
                if (startTimeNextLayout <= mPlayheadTime)
                {
                    stepToNextLayout();
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
    }
    
    std::map<int, TimelineContentInfo> GridLayoutTimeline::getRenderContent(ContentProvider *contentProvider)
    {
        std::map< int, TimelineContentInfo > returnContent;
        
        vector<ScreenRegion> transitionRegions;
        vector<ScreenRegion> compareRegions;
        vector<ScreenRegion> returnRegions;
        
        float curTransitionScale = mTransitionAmt;
        
        if (mTransitionAmt < 1.0 && mIdxPrevLayout != -1)
        {
            // If there's a previous region, they each get 1/2 of the
            // allotted transition time.
            
            float prevTransitionScale = 1.0 - std::min(mTransitionAmt / 0.5f, 1.0f);
            float nextTransitionScale = std::max((mTransitionAmt - 0.5f) / 0.5f, 0.0f);
            
            if (nextTransitionScale > 0.0)
            {
                // Next layout is transitioning.
                transitionRegions = mGridLayouts[mIdxCurrentLayout].getRegions();
                compareRegions = mGridLayouts[mIdxPrevLayout].getRegions();
                
                curTransitionScale = nextTransitionScale;
            }
            else
            {
                // Previous layout is transitioning.
                transitionRegions = mGridLayouts[mIdxPrevLayout].getRegions();
                compareRegions = mGridLayouts[mIdxCurrentLayout].getRegions();
                
                curTransitionScale = prevTransitionScale;
            }
        }
        else
        {
            // Next layout is transitioning. There are no previous regions.
            transitionRegions = mGridLayouts[mIdxCurrentLayout].getRegions();
        }

        int numRegions = transitionRegions.size();
        int numCompareRegions = compareRegions.size();
        for (int i = 0; i < numRegions; ++i)
        {
            ScreenRegion transitionReg = transitionRegions[i];
            Rectf rA = transitionReg.rect;
            float rectScale = curTransitionScale;
            
            for (int j = 0; j < numCompareRegions; ++j)
            {
                ScreenRegion compareReg = compareRegions[j];
                Rectf rB = compareReg.rect;
                
                if (compareReg.isActive &&
                    rectCompare(rA, rB))
                {
                    // No transition. Just draw at full blast.
                    rectScale = 1.0f;
                    break;
                }
            }
            
            //std::pair<ci::Rectf, RenderableContentRef> returnItem;
            TimelineContentInfo returnItem;
            
            // Now, create the new rect.
            Vec2f rectCenter = rA.getCenter();
            Vec2f rectHalfSize = rA.getSize() * 0.5f * rectScale;
            Rectf transitionRect(rectCenter - rectHalfSize,
                                 rectCenter + rectHalfSize);
            // returnItem.first = transitionRect;
            returnItem.rect = transitionRect;
            
            // Append content.
            // returnItem.second = contentProvider->contentForKey(transitionReg.contentKey);
            returnItem.contentKey = transitionReg.contentKey;
            returnItem.contentRef = contentProvider->contentForKey(transitionReg.contentKey);
            
            returnContent[transitionReg.timelineID] = returnItem;
        }
        
        return returnContent;
    }
    
    
}