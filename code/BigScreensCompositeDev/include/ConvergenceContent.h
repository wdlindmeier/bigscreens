//
//  ConvergenceContent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/7/13.
//
//

#pragma once

#include "cinder/gl/gl.h"
#include "cinder/gl/Shader.h"
#include "SharedTypes.hpp"
#include "Utilities.hpp"
#include "GridLayout.h"
#include "SceneWindow.hpp"
#include "cinder/Surface.h"
#include "cinder/Text.h"
#include "GridLayout.h"
#include "OutLineBorder.hpp"
#include "cinder/Easing.h"
#include "cinder/Rand.h"

namespace bigscreens
{

    enum TransitionStyle
    {
        TRANSITION_ALPHA = 0,
        TRANSITION_EXPAND,
        TRANSITION_FADE
    };
    
    class ConvergenceContent : public RenderableContent
    {
        
    public:
        
        ConvergenceContent();
        ~ConvergenceContent(){}
        
        void load();
        void setMSElapsed(const long msElapsedConvergence);
        void update(const float totalTimelineProgress);
        void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
        void reset(const GridLayout & previousLayout);
		ci::Camera & getCamera();
        void setContentRect(const ci::Rectf & rect);
        void setFramesRendered(const long long numFramesRendered);
        void setConvergenceTankContent(RenderableContentRef content);
        
    protected:
        
        float getScalarMergeProgress();

        void renderWithFadeTransition(const ci::Vec2i screenOffset,
                                      const ci::Rectf & rect,
                                      const float alpha);

    private:
        
        GridLayout mLayout;
        ci::CameraPersp mCam;
        RenderableContentRef mContent;
        ci::Rectf mContentRect;
        std::vector<CameraOrigin> mCameraOrigins;
        OutLineBorderRef mOutLine;
        long mMSElapsedConvergence;
        ci::EaseInOutCubic mCamEase;
        float mTotalTimelineProgress;
        ci::Rand mRand;
        long mNumFramesPrevLayoutRendered;

    };

}