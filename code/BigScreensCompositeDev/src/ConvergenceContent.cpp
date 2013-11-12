//
//  ConvergenceContent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/7/13.
//
//

#include "ConvergenceContent.h"
#include "TankConvergenceContent.h"
#include "BigScreensConstants.h"

using namespace std;
using namespace ci;
using namespace ci::gl;
using namespace ci::app;
using namespace bigscreens;

extern long MSCamerasConverge = 1000;
extern long MSConvergeBeforeCameraMerge = 1000;

ConvergenceContent::ConvergenceContent() : mMSElapsedConvergence(0)
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

void ConvergenceContent::setMSElapsed(const long msElapsedConvergence)
{
    mMSElapsedConvergence = msElapsedConvergence;
}

void ConvergenceContent::reset(const GridLayout & previousLayout)
{
    shared_ptr<TankConvergenceContent> content = static_pointer_cast<TankConvergenceContent>(mContent);
    content->reset();

    mLayout = previousLayout;
    mCameraOrigins.clear();
    int i = 0;
    int regionCount = mLayout.getRegions().size();
    for (ScreenRegion region : mLayout.getRegions())
    {
        CameraOrigin orig = TankConvergenceContent::cameraForTankConvergence(i,
                                                                             regionCount,
                                                                             MSConvergeBeforeCameraMerge,
                                                                             mContentRect.getSize(),
                                                                             region.rect);
        mCameraOrigins.push_back(orig);
        
        ++i;
    }
}

void ConvergenceContent::update()
{
    shared_ptr<TankConvergenceContent> content = static_pointer_cast<TankConvergenceContent>(mContent);
    content->setFramesRendered(mNumFramesRendered);
}

ci::Camera & ConvergenceContent::getCamera()
{
    return mCam;
}

float ConvergenceContent::getScalarMergeProgress()
{
    return (float)(mMSElapsedConvergence - MSConvergeBeforeCameraMerge) / (float)MSCamerasConverge;
}

void ConvergenceContent::render(const ci::Vec2i & screenOffset,
                                const ci::Rectf & contentRect)
{

    gl::clear( Color( 0.0, 0.0, 0.0 ) );
    
    gl::enableAlphaBlending();
    
    vector<ScreenRegion> regions = mLayout.getRegions();
    int regionCount = regions.size();
    
    // Time based
    float progress = getScalarMergeProgress();
    float amtRemaining = 1.0 - std::min<float>(progress, 1.0f);

    // Weigh
    
    // TODO: Use an ease-in ease-out function
    amtRemaining *= amtRemaining;
    
    // Create the target cam position
    float scalarProgress = 1.0 - std::min<float>(progress, 1.0f);
    float finalOffset = scalarProgress * scalarProgress;
    float camDist = 3000 + (6000 * finalOffset);
    float camY = camDist * 0.5;
    float camZ = camDist;
    float camX = camDist;
    
    CameraOrigin finalOrigin;
    finalOrigin.eye = Vec3f( camX, camY, camZ );
    finalOrigin.target = Vec3f( 0.f, -1000.f, 0.f );
    finalOrigin.camShift = Vec2f(0,0);

    // NOTE: This content starts in the previous layout so we're adding render frames
    // mContent->setFramesRendered(kNumFramesConvergeBeforeCameraMerge + mNumFramesRendered);
    
    shared_ptr<TankConvergenceContent> tanks = static_pointer_cast<TankConvergenceContent>(mContent);
    tanks->setMSElapsed(mMSElapsedConvergence);
    
    float fullAspectRatio = mContentRect.getWidth() / mContentRect.getHeight();
    mCam.setPerspective( 45.0f, fullAspectRatio, .01, 40000 );

    ci::gl::enable( GL_SCISSOR_TEST );
    
    // Draw the "background" image that fades in at the end.
    const static float kFinalSceneProgressBegin = 0.75;
    if (progress >= kFinalSceneProgressBegin &&
        mTransitionStyle == TRANSITION_FADE)
    {
        // This uses the final cam
        tanks->update([=](CameraPersp & cam)
        {
            cam.setAspectRatio(fullAspectRatio);
            cam.lookAt( finalOrigin.eye, finalOrigin.target );
        });
        // Subtracting the progress begin so the alpha is 0 when it comes in.
        float finalSceneAlpha = std::min<float>(1.0, (progress*progress*progress*progress) - kFinalSceneProgressBegin);
        renderWithFadeTransition(screenOffset,
                                 mContentRect,
                                 finalSceneAlpha);
    }

    float fadeAlpha = 1.0 + std::max<float>(1.0f-progress, -1.0);
    bool shouldDrawRegions = mTransitionStyle != TRANSITION_FADE || fadeAlpha > 0.01;

    // If we're using the FADE style, the sub-regions shouldn't be drawn if they're invisible
    if (shouldDrawRegions)
    {
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
                        renderWithFadeTransition(screenOffset,
                                                 region.rect,
                                                 fadeAlpha);
                        break;
                }
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
    
    float progress = getScalarMergeProgress();
    float alpha = pow(progress, 6); //(float)mNumFramesRendered / (kNumFramesCamerasConverge * 6.0f);
    static_pointer_cast<TankConvergenceContent>(mContent)->render(screenOffset, rect, alpha);

}

void ConvergenceContent::renderWithExpandTransition(const ci::Vec2i screenOffset, const Rectf & rect)
{
    Vec2i contentSize = mContentRect.getSize();

    ci::gl::viewport(-screenOffset.x,
                     -screenOffset.y,
                     contentSize.x,
                     contentSize.y);

    float progress = getScalarMergeProgress();
    float linearProgress = std::min<float>(1.0, progress);
    
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

    float progress = getScalarMergeProgress();
    float linearProgress = std::min<float>(1.0, progress);
    
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

