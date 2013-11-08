//
//  GridLayoutTimeline.h
//
//  Created by Ryan Bartley on 10/14/13.
//
//

#pragma once

#include "SharedTypes.hpp"
#include "cinder/Cinder.h"
#include "GridLayout.h"

namespace bigscreens
{
    
    typedef std::shared_ptr<class GridLayoutTimeline> GridLayoutTimelineRef;
    
    // This is the container class for all GridLayouts
    // it will be where all transitional and current
    // GridLayout math will be constructed
    
    class GridLayoutTimeline
    {
    public:
        
        GridLayoutTimeline( const cinder::fs::path & filePath, float scale = 1.0f );
        ~GridLayoutTimeline() {}

        // Neé "tick"
        // NOTE: We don't actually want to use "future" since the transition amount
        // requires a knowledge of the current time. Transitions are absolute milliseconds.
        long long update();
        
        std::map<int, TimelineContentInfo> getRenderContent(ContentProvider *contentProvider,
                                                            bool shouldTransition=true);
        
        std::vector<GridLayout> & getGridLayouts(){ return mGridLayouts; };
        
        // Playback control
        void stepToNextLayout();
        void stepToPreviousLayout();
        void restart();
        void play();
        void pause();
        void newLayoutWasSet();
        void reload();
        void loadAllGrids();
        void assignTimelineIDs();
        
        bool isPlaying();
        int getCurrentFrame(){ return mIdxCurrentLayout; };
        int getCurrentRegionCount();
        
    private:
        
        cinder::fs::path mLoadPath;
        
        std::vector<GridLayout> mGridLayouts;
        int mIdxCurrentLayout;
        int mIdxPrevLayout;
        
        float mTransitionAmt;
        long long mStartTime;
        long long mPlayheadTime;
        long long mLastFrameTime;
        long long mTotalDuration;
        float mPlaybackSpeed;
        bool mIsPlaying;
        float mScale;
        
    };
    
}
