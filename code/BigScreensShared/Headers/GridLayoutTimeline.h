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
        void update();
        
        // We're passing back the region and the content as a pair.
        // No need to copy/send the whole layout.
        // TODO: Make a shared content provider.
        std::vector< std::pair<ci::Rectf, RenderableContentRef> > getRenderContent(ContentProvider *contentProvider);

        // Playback control
        void stepToNextLayout();
        void stepToPreviousLayout();
        void restart();
        void play();
        void pause();
        void newLayoutWasSet();
        void reload();
        void loadAllGrids();
        
        bool isPlaying();
        
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
