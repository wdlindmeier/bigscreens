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
        
        void load(const TransitionStyle style);
        void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
        void reset(const GridLayout & previousLayout);
		ci::Camera & getCamera();
        void setContentRect(const ci::Rectf & rect);
        
    protected:
        
        void renderWithAlphaTransition(const ci::Vec2i screenOffset,
                                       const ci::Rectf & rect);
        void renderWithExpandTransition(const ci::Vec2i screenOffset,
                                        const ci::Rectf & rect);
        void renderWithFadeTransition(const ci::Vec2i screenOffset,
                                      const ci::Rectf & rect,
                                      const float alpha);

    private:
        
        GridLayout mLayout;
        ci::CameraPersp mCam;
        TransitionStyle mTransitionStyle;
        RenderableContentRef mContent;
        ci::Rectf mContentRect;
        std::vector<CameraOrigin> mCameraOrigins;
        OutLineBorderRef mOutLine;

    };

}