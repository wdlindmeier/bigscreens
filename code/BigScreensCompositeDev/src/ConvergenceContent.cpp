//
//  ConvergenceContent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/7/13.
//
//

#include "ConvergenceContent.h"
#include "TankConvergenceContent.h"

using namespace std;
using namespace ci;
using namespace ci::gl;
using namespace ci::app;
using namespace bigscreens;

ConvergenceContent::ConvergenceContent()
{
}

void ConvergenceContent::setContentRect(const ci::Rectf & rect)
{
    mContentRect = rect;
}

void ConvergenceContent::load(const TransitionStyle style)
{
    TankConvergenceContent *content = new TankConvergenceContent();
    content->load("T72.obj");
    content->reset();
    mContent = RenderableContentRef(content);
    mTransitionStyle = style;
    mOutLine = OutLineBorderRef(new OutLineBorder());
}

void ConvergenceContent::reset(const GridLayout & previousLayout)
{
    shared_ptr<TankConvergenceContent> content = static_pointer_cast<TankConvergenceContent>(mContent);
    content->reset();

    mLayout = previousLayout;
    mCameraOrigins.clear();
    int i = 0;
    
    for (ScreenRegion region : mLayout.getRegions())
    {
        CameraOrigin orig;
        
        TankOrientation orient = content->positionForTankWithProgress(i, kNumFramesConvergeBeforeCameraMerge);
        Vec3f tankPos = orient.position;

        // TMP: Hard-coding the positions for now
        orig.eye = Vec3f(tankPos.x + ((200 * i) - 2000), //((int)(arc4random() % 4000) - 2000.0f),
                         tankPos.y + 800, //(int)(arc4random() % 1000),
                         tankPos.z + ((200 * i) - 2000)); //(int)(arc4random() % 4000) - 2000.0f);

        /*
        // Randomly oreient the eye around the tank
        orig.eye = Vec3f(tankPos.x + ((int)(arc4random() % 4000) - 2000.0f),
                         tankPos.y + (int)(arc4random() % 1000),
                         tankPos.z + (int)(arc4random() % 4000) - 2000.0f);
        */
        
        // Look at the tank
        orig.target = Vec3f(tankPos.x, 100, tankPos.z);
        
        // A horizontal lens shift of 1 (-1) will shift the view right (left) by half the width of the viewport.
        // A vertical lens shift of 1 (-1) will shift the view up (down) by half the height of the viewport.
        Vec2f viewCenter = mContentRect.getCenter();
        Vec2f regCenter = region.rect.getCenter();
        float horizShift = 1.0 - (regCenter.x / viewCenter.x);
        float vertShift = 1.0 - (regCenter.y / viewCenter.y);
        // TMP: Hard-coding for test
        orig.camShift = Vec2f(horizShift, vertShift);
        
        mCameraOrigins.push_back(orig);
        
        ++i;
    }

}

ci::Camera & ConvergenceContent::getCamera()
{
    return mCam;
}

void ConvergenceContent::render(const ci::Vec2i & screenOffset,
                                const ci::Rectf & contentRect)
{

    gl::clear( Color( 0.0, 0.0, 0.0 ) );
    
    gl::enableAlphaBlending();
    
    vector<ScreenRegion> regions = mLayout.getRegions();
    int regionCount = regions.size();
    
    float progress = (float)mNumFramesRendered / (float)kNumFramesCamerasConverge;
    float amtRemaining = 1.0 - std::min<float>(progress, 1.0f);
    // Weigh
    amtRemaining *= amtRemaining;
    
    // Create the target cam position
    float scalarProgress = 1.0 - std::min<float>(progress, 1.0f);
    float finalOffset = scalarProgress * scalarProgress;
    float camDist = 4000 + (6000 * finalOffset);
    float camY = camDist * 0.25;
    float camZ = camDist;
    float camX = camDist;
    
    CameraOrigin finalOrigin;
    finalOrigin.eye = Vec3f( camX, camY, camZ );
    finalOrigin.target = Vec3f( 0.f, -1750.f, 0.f );
    finalOrigin.camShift = Vec2f(0,0);

    // NOTE: This content starts in the previous layout so we're adding render frames
    mContent->setFramesRendered(kNumFramesConvergeBeforeCameraMerge + mNumFramesRendered);
    
    shared_ptr<TankConvergenceContent> tanks = static_pointer_cast<TankConvergenceContent>(mContent);
    
    float fullAspectRatio = mContentRect.getWidth() / mContentRect.getHeight();
    mCam.setPerspective( 45.0f, fullAspectRatio, .01, 40000 );

    ci::gl::enable( GL_SCISSOR_TEST );
    
    // Draw the "background" image that fades in at the end.
    const static float kMinFinalAlpha = 0.75;
    if (progress >= kMinFinalAlpha && mTransitionStyle == TRANSITION_FADE)
    {
        // This uses the final cam
        tanks->update([=](CameraPersp & cam)
        {
            cam.setAspectRatio(fullAspectRatio);
            cam.lookAt( finalOrigin.eye, finalOrigin.target );
        });
        
        renderWithFadeTransition(screenOffset, mContentRect, (progress*progress*progress*progress) - kMinFinalAlpha);
    }
    
    // Draw each sub-region that converge
    for (int i = 0; i < regionCount; ++i)
    {
        ScreenRegion & region = regions[i];
        
        if (rectsOverlap(region.rect, mContentRect))
        {
            CameraOrigin orig = mCameraOrigins[i];
            
            // Lerp the region cam with the final cam
            Vec3f eyeOffset = orig.eye - finalOrigin.eye;
            Vec3f currentEye = finalOrigin.eye + (eyeOffset * amtRemaining);
            
            Vec3f targetOffset = orig.target - finalOrigin.target;
            Vec3f currentTarget = finalOrigin.target + (targetOffset * amtRemaining);
            
            Vec2f camShiftOffset = orig.camShift - finalOrigin.camShift;
            Vec2f currentCamShift = finalOrigin.camShift + (camShiftOffset * amtRemaining);
            
            tanks->update([=](CameraPersp & cam)
            {
                cam.setAspectRatio(fullAspectRatio);
                cam.lookAt( currentEye, currentTarget );
                cam.setLensShift(currentCamShift);
            });
            
            switch (mTransitionStyle)
            {
                case TRANSITION_ALPHA:
                    renderWithAlphaTransition(screenOffset, region.rect);
                    break;
                case TRANSITION_EXPAND:
                    renderWithExpandTransition(screenOffset, region.rect);
                    break;
                case TRANSITION_FADE:
                    renderWithFadeTransition(screenOffset, region.rect, 1.0 + std::max<float>(1.0-progress, 0.0));
                    break;
            }
        }
    }

    ci::gl::disable( GL_SCISSOR_TEST );
}

void ConvergenceContent::renderWithAlphaTransition(const ci::Vec2i screenOffset, const Rectf & rect)
{
    Vec2i contentSize = mContentRect.getSize();

    ci::gl::scissor(-screenOffset.x,//-screenOffset.x,
                    0,
                    contentSize.x,
                    contentSize.y);

    ci::gl::viewport(-screenOffset.x,
                     -screenOffset.y,
                     contentSize.x,
                     contentSize.y);
    
    //Vec2i offset(0,0);
    
    float alpha = (float)mNumFramesRendered / (kNumFramesCamerasConverge * 6.0f);
    static_pointer_cast<TankConvergenceContent>(mContent)->render(screenOffset, rect, alpha);

}

void ConvergenceContent::renderWithExpandTransition(const ci::Vec2i screenOffset, const Rectf & rect)
{
    Vec2i contentSize = mContentRect.getSize();

    ci::gl::viewport(-screenOffset.x,
                     -screenOffset.y,
                     contentSize.x,
                     contentSize.y);

    float linearProgress = std::min<float>(1.0, (float)mNumFramesRendered / (float)kNumFramesCamerasConverge);
    
    float x = rect.x1 * (1.0 - linearProgress);
    x -= screenOffset.x;
    
    float minY = contentSize.y - (rect.y2 - screenOffset.y);
    float maxY = 0;
    float y = minY + ((maxY - minY) * linearProgress);
    
    float widthDelta = contentSize.x - rect.getWidth();
    float width = rect.getWidth() + (linearProgress * widthDelta);
    
    float heightDelta = contentSize.y - rect.getHeight();
    float height = rect.getHeight() + (linearProgress * heightDelta);
    
    ci::gl::scissor(x,
                    y,
                    width,
                    height);
    
    float minAlpha = 1;//0.15;
    float alphaTravel = 1.0 - minAlpha;
    float alpha = minAlpha + ((1.0-linearProgress) * alphaTravel);
    static_pointer_cast<TankConvergenceContent>(mContent)->render(screenOffset, rect, alpha);
    
    float outlineAlpha = 1.0 - linearProgress;
    outlineAlpha *= outlineAlpha;
    
    ci::gl::viewport(x,
                     y,
                     width,
                     height);

    mOutLine->setColor(ColorAf(1.0,1.0,1.0,outlineAlpha));
    mOutLine->render();
}

void ConvergenceContent::renderWithFadeTransition(const ci::Vec2i screenOffset,
                                                  const Rectf & rect,
                                                  const float alpha)
{
    // Clip the viewport, similar to SceneWindow
    Vec2i contentSize = mContentRect.getSize();

    float linearProgress = std::min<float>(1.0, (float)mNumFramesRendered / (float)kNumFramesCamerasConverge);
    
    ci::gl::viewport(-screenOffset.x,
                     -screenOffset.y,
                     contentSize.x,
                     contentSize.y);

    ci::gl::scissor(rect.x1 - screenOffset.x,
                    contentSize.y - rect.y2 - screenOffset.y,
                    rect.getWidth(),
                    rect.getHeight());

    static_pointer_cast<TankConvergenceContent>(mContent)->render(screenOffset, rect, alpha);

    float outlineAlpha = 1.0 - linearProgress;
    outlineAlpha *= outlineAlpha;

    ci::gl::viewport(rect.x1 - screenOffset.x,
                     contentSize.y - rect.y2 - screenOffset.y,
                     rect.getWidth(),
                     rect.getHeight());

    mOutLine->setColor(ColorAf(1.0,1.0,1.0,outlineAlpha));
    mOutLine->render();
}

