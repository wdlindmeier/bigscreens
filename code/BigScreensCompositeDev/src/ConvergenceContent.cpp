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
//extern float MaxExplosionScale;// = kDefaultExplosionScale;

ConvergenceContent::ConvergenceContent() : mMSElapsedConvergence(0)
{
}

void ConvergenceContent::setContentRect(const ci::Rectf & rect)
{
    mContentRect = rect;
}

void ConvergenceContent::load()
{
    TankConvergenceContent *content = new TankConvergenceContent();
    content->load();
    content->reset();
    mContent = RenderableContentRef(content);
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
}

void ConvergenceContent::update(const float totalTimelineProgress)
{
    mTotalTimelineProgress = totalTimelineProgress;
    shared_ptr<TankConvergenceContent> content = static_pointer_cast<TankConvergenceContent>(mContent);
    content->resetPositions();
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
    
    gl::enableAdditiveBlending();
    
    vector<ScreenRegion> regions = mLayout.getRegions();
    int regionCount = regions.size();
    
    // Time based
    float progress = getScalarMergeProgress();
    // Weigh
    float easeProgress = mCamEase(progress);

    float amtRemaining = 1.0 - std::min<float>(easeProgress, 1.0f);

    // Create the target cam position
    float camDist = 3500 + (6000 * amtRemaining);
    float camY = camDist * 0.5;
    float camZ = camDist;
    float camX = camDist;
    
    CameraOrigin finalOrigin;
    finalOrigin.eye = Vec3f( camX, camY, camZ );
    finalOrigin.target = Vec3f( 0.f, -1000.f, 0.f );
    finalOrigin.camShift = Vec2f(0,0);

    shared_ptr<TankConvergenceContent> tanks = static_pointer_cast<TankConvergenceContent>(mContent);
    tanks->setMSElapsed(mMSElapsedConvergence);
    
    float fullAspectRatio = mContentRect.getWidth() / mContentRect.getHeight();
    mCam.setPerspective( 45.0f, fullAspectRatio, .01, 40000 );

    ci::gl::enable( GL_SCISSOR_TEST );
    
    // Draw the "background" image that fades in at the end.
    const static float kFinalSceneProgressBegin = 0.75;
    
    float finalSceneAlpha = (progress*progress*progress*progress) - kFinalSceneProgressBegin;
    float fadeAlpha = 1.5 - finalSceneAlpha;
    finalSceneAlpha = std::min<float>(1.0, finalSceneAlpha);
    bool shouldDrawRegions = fadeAlpha > 0.01;

    if (progress >= kFinalSceneProgressBegin && finalSceneAlpha > 0.0f)
    {
        static const int kLastContentID = 9999;
        mContent->setFrameContentID(kLastContentID);

        if (!shouldDrawRegions &&
            (arc4random() % 100) < 20)
        {
            int fireTankIdx = arc4random() % kNumTanksConverging;
            tanks->fireTankGun(fireTankIdx);
        }

        // This uses the final cam
        // TODO: Use a dumb tank multiplier
        tanks->update([=](CameraPersp & cam, DumbTankRef & tank)
        {
            cam.setAspectRatio(fullAspectRatio);
            cam.lookAt( finalOrigin.eye, finalOrigin.target );
        });

        renderWithFadeTransition(screenOffset,
                                 mContentRect,
                                 finalSceneAlpha);
    }

    // If we're using the FADE style, the sub-regions shouldn't be drawn if they're invisible
    if (shouldDrawRegions)
    {
        // Only draw the subcontent w/ in this screen.
        // NOTE: This assumes the mContentRect is the whole size of the screen.
        Rectf screenRect(screenOffset.x,
                         screenOffset.y,
                         screenOffset.x + (mContentRect.getWidth() / kNumScreens),
                         screenOffset.y + mContentRect.getHeight());
                         
        // Draw each sub-region that converge
        for (int i = 0; i < regionCount; ++i)
        {
            ScreenRegion & region = regions[i];
            mContent->setFrameContentID(region.timelineID);
            
            if (rectsOverlap(region.rect, screenRect))
            // if (rectsOverlap(region.rect, mContentRect))
            {
                // CameraOrigin orig = mCameraOrigins[i];
                
                // Rather than getting the stale origin, blend it with the current tank origin
                CameraOrigin orig = TankConvergenceContent::cameraForTankConvergence(i,
                                                                                     regionCount,
                                                                                     mMSElapsedConvergence,
                                                                                     mContentRect.getSize(),
                                                                                     region.rect);
                
                // Lerp the region cam with the final cam
                Vec3f eyeOffset = orig.eye - finalOrigin.eye;
                Vec3f currentEye = finalOrigin.eye + (eyeOffset * amtRemaining);
                
                Vec3f targetOffset = orig.target - finalOrigin.target;
                Vec3f currentTarget = finalOrigin.target + (targetOffset * amtRemaining);
                
                Vec2f camShiftOffset = orig.camShift - finalOrigin.camShift;
                Vec2f currentCamShift = finalOrigin.camShift + (camShiftOffset * amtRemaining);
                
                tanks->update([=](CameraPersp & cam, DumbTankRef & tank)
                {
                    cam.setAspectRatio(fullAspectRatio);
                    cam.lookAt( currentEye, currentTarget );
                    cam.setLensShift(currentCamShift);
                });
                
                renderWithFadeTransition(screenOffset,
                                         region.rect,
                                         fadeAlpha);
            }
        }
    }

    ci::gl::disable( GL_SCISSOR_TEST );

    // A simple fade out. Tweak as necessary.
    const static float kPercentFadeOut = 0.05;
    if (mTotalTimelineProgress > (1.0 - kPercentFadeOut))
    {
        float fadeOutAmt = (1.0 - mTotalTimelineProgress) / kPercentFadeOut;
        gl::setMatricesWindow(contentRect.getWidth(), contentRect.getHeight());
        gl::bindStockShader(ShaderDef().color());
        gl::enableAlphaBlending();
        gl::color(ColorAf(0,0,0,1.0-fadeOutAmt));
        gl::setDefaultShaderVars();
        gl::drawSolidRect(Rectf(0,0,contentRect.getWidth(), contentRect.getHeight()));
    }
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

    gl::enableAlphaBlending();
    
    float outlineAlpha = 1.0 - linearProgress;
    outlineAlpha *= outlineAlpha;
    
    outlineAlpha = alpha;

    ci::gl::viewport(rect.x1 - screenOffset.x,
                     contentSize.y - rect.y2 - screenOffset.y,
                     rect.getWidth(),
                     rect.getHeight());

    mOutLine->setColor(ColorAf(1.0,1.0,1.0,outlineAlpha));
    mOutLine->render();
#if IS_IAC
    // Drawing a second, inner line at IAC because we're missing some edges
    ci::gl::viewport(rect.x1 - screenOffset.x + 1,
                     contentSize.y - rect.y2 - screenOffset.y + 1,
                     rect.getWidth() - 2,
                     rect.getHeight() - 2);
    mOutLine->render();
#endif

}

