#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Shader.h"
#include "SharedTypes.hpp"
#include "Utilities.hpp"
#include "GridLayout.h"
#include "SampleRenderable.h"
#include "SceneWindow.hpp"
#include "cinder/Surface.h"
#include "cinder/Text.h"
#include "OutLineBorder.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace bigscreens;

enum TransitionStyle
{
    TRANSITION_ALPHA = 0,
    TRANSITION_EXPAND,
    TRANSITION_FADE
};

struct CameraOrigin
{
    ci::Vec3f eye;
    ci::Vec3f target;
};

class ConvergenceTestApp : public AppNative {
  public:
    void prepareSettings(Settings *settings);
	void setup();
    void resetCameras();
	void mouseDown(MouseEvent event);
    void keyDown(KeyEvent event);
    //void mouseDown( MouseEvent event );
	void update();
	void draw();
    void renderWithAlphaTransition(Rectf & rect);
    void renderWithExpandTransition(Rectf & rect);
    void renderWithFadeTransition(Rectf & rect, float alpha);
    
    GridLayout mLayout;
    RenderableContentRef mContent;
    long mProgressFrames;
    vector<CameraOrigin> mCameraOrigins;
    OutLineBorderRef     mOutLine;
    
    TransitionStyle mTransitionStyle;
    
    gl::TextureRef mTextureKey;
};

void ConvergenceTestApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(960, 270);
}

void ConvergenceTestApp::setup()
{
    mLayout = GridLayout::load(SharedGridAssetPath(true) / "8039292.grid",
                               0.25f);
    SampleRenderable *content = new SampleRenderable();
    content->load();
    mContent = RenderableContentRef(content);
    mProgressFrames = 0;
    resetCameras();
    mTransitionStyle = TRANSITION_ALPHA;
    mOutLine = OutLineBorderRef(new OutLineBorder());

    
    Font font("Helvetica Neue", 12);
    Surface frameSurf = renderString("KEY: 1) Alpha 2) Expand 3) Fade",
                                     font,
                                     ColorAf(0,1,1,1));
    gl::Texture *frameTex = new gl::Texture(frameSurf);
    mTextureKey = gl::TextureRef(frameTex);
}

void ConvergenceTestApp::resetCameras()
{
    mCameraOrigins.clear();
    int i = 0;
//    int regionCount = mLayout.getRegions().size();
    for (ScreenRegion region : mLayout.getRegions())
    {
        CameraOrigin orig;
        
        // Random eye
        orig.eye = Vec3f((int)(arc4random() % 4000) - 2000.0f,
                         (int)(arc4random() % 1000),
                         (int)(arc4random() % 4000) - 2000.0f);

        // Random Target
        orig.target = Vec3f((int)(arc4random() % 4000) - 2000.0f,
                            (int)(arc4random() % 1000),
                            (int)(arc4random() % 4000) - 2000.0f);
        
        mCameraOrigins.push_back(orig);
        ++i;
    }
}

void ConvergenceTestApp::keyDown(KeyEvent event)
{
    char key = event.getChar();
    if (key == '1')
    {
        mTransitionStyle = TRANSITION_ALPHA;
        console() << "Transition Alpha\n";
    }
    else if (key == '2')
    {
        mTransitionStyle = TRANSITION_EXPAND;
        console() << "Transition Expand\n";
    }
    else if (key == '3')
    {
        mTransitionStyle = TRANSITION_FADE;
        console() << "Transition Fade\n";
    }
    resetCameras();
    mProgressFrames = 0;
}

void ConvergenceTestApp::mouseDown( MouseEvent event )
{
    resetCameras();
    mProgressFrames = 0;
}

void ConvergenceTestApp::update()
{
    mProgressFrames++;
}

void ConvergenceTestApp::draw()
{
    gl::clear( Color( 0.0, 0.0, 0.0 ) );

    gl::enableAlphaBlending();
    
    vector<ScreenRegion> regions = mLayout.getRegions();
    int regionCount = regions.size();
    
    float progress = (float)mProgressFrames / (float)kFramesFullTransition;
    float linearAmtRemaining = std::max<float>(0, 1.0-progress);
    float amtRemaining = 1.0 - std::min<float>(progress, 1.0f);

    amtRemaining *= amtRemaining;

    // TMP: First, just try getting them to share a scene
    // amtRemaining = 0;

    CameraOrigin finalOrigin;
    finalOrigin.eye = Vec3f( 1000, 400, 0.1 );
    finalOrigin.target = Vec3f( 0.1, 100, 0.1 );
    
    mContent->setFramesRendered(mProgressFrames);

    if (progress >= 1.0 && mTransitionStyle == TRANSITION_FADE)
    {
        static_pointer_cast<SampleRenderable>(mContent)->update([=](CameraPersp & cam)
        {
            cam.setAspectRatio(getWindowAspectRatio());
            cam.lookAt( finalOrigin.eye, finalOrigin.target );
        });

        Rectf rect(0, 0, getWindowWidth(), getWindowHeight());
        renderWithFadeTransition(rect, (progress*progress*progress*progress) - 1.0);
    }

    for (int i = 0; i < regionCount; ++i)
    {
        ScreenRegion & region = regions[i];
        
        CameraOrigin orig = mCameraOrigins[i];
        
        // Update the cam
        Vec3f eyeOffset = orig.eye - finalOrigin.eye;
        Vec3f currentEye = finalOrigin.eye + (eyeOffset * amtRemaining);
        
        Vec3f targetOffset = orig.target - finalOrigin.target;
        Vec3f currentTarget = finalOrigin.target + (targetOffset * amtRemaining);

        static_pointer_cast<SampleRenderable>(mContent)->update([=](CameraPersp & cam)
        {
            cam.setAspectRatio(getWindowAspectRatio());
            cam.lookAt( currentEye, currentTarget );
        });
        
        
        switch (mTransitionStyle)
        {
            case TRANSITION_ALPHA:
                renderWithAlphaTransition(region.rect);
                break;
            case TRANSITION_EXPAND:
                renderWithExpandTransition(region.rect);
                break;
            case TRANSITION_FADE:
                renderWithFadeTransition(region.rect, 1.0 + std::max<float>(1.0-progress, 0.0));
                break;
        }
    }
    
    gl::setMatricesWindow(getWindowSize());
    Vec2i windowSize = getWindowSize();
    ci::gl::viewport(0,0,windowSize.x,windowSize.y);
    gl::draw(mTextureKey, Rectf(10, 10, 10+mTextureKey->getWidth(), 10 + mTextureKey->getHeight()));
    
}

void ConvergenceTestApp::renderWithAlphaTransition(Rectf & rect)
{
    // Clip the viewport, similar to SceneWindow
    Vec2i windowSize = getWindowSize();
    ci::gl::viewport(0,0,windowSize.x,windowSize.y);

    Vec2i offset(0,0);
    
    float alpha = (float)mProgressFrames / (kFramesFullTransition*6.0f);
    static_pointer_cast<SampleRenderable>(mContent)->render(offset, alpha);
}

void ConvergenceTestApp::renderWithExpandTransition(Rectf & rect)
{
    // Clip the viewport, similar to SceneWindow
    Vec2i windowSize = getWindowSize();
    ci::gl::viewport(0,0,windowSize.x,windowSize.y);

    Vec2i offset(0,0);
    
    float linearProgress = std::min<float>(1.0, (float)mProgressFrames / (float)kFramesFullTransition);
    
    float x = (rect.x1 - offset.x) * (1.0 - linearProgress);
    
    float minY = windowSize.y - (rect.y2 - offset.y);
    float maxY = 0;
    float y = minY + ((maxY - minY) * linearProgress);
    
    float widthDelta = windowSize.x - rect.getWidth();
    float width = rect.getWidth() + (linearProgress * widthDelta);

    float heightDelta = windowSize.y - rect.getHeight();
    float height = rect.getHeight() + (linearProgress * heightDelta);
    
    ci::gl::enable( GL_SCISSOR_TEST );
    ci::gl::scissor(x,
                    y,
                    width,
                    height);

    float minAlpha = 1;//0.15;
    float alphaTravel = 1.0 - minAlpha;
    float alpha = minAlpha + ((1.0-linearProgress) * alphaTravel);
    static_pointer_cast<SampleRenderable>(mContent)->render(offset, alpha);
    ci::gl::disable( GL_SCISSOR_TEST );
    
    float outlineAlpha = 1.0 - linearProgress;
    outlineAlpha *= outlineAlpha;
    
    ci::gl::viewport(x,
                     y,
                     width,
                     height);
    mOutLine->setColor(ColorAf(1.0,1.0,1.0,outlineAlpha));
    mOutLine->render();
}

void ConvergenceTestApp::renderWithFadeTransition(Rectf & rect, float alpha)
{
    Vec2i windowSize = getWindowSize();
    ci::gl::viewport(0,0,windowSize.x,windowSize.y);
    Vec2i offset(0,0);

    //float linearProgress = std::min<float>(1.0, (float)mProgressFrames / (float)kFramesFullTransition);
    
    ci::gl::enable( GL_SCISSOR_TEST );
    ci::gl::scissor(rect.x1 - offset.x,
                    windowSize.y - rect.y2 - offset.y,
                    rect.getWidth(),
                    rect.getHeight());
    /*
    gl::bindStockShader(gl::ShaderDef().color());
    gl::color(0, 0, 0, alpha);
    gl::setMatricesWindow(windowSize.x, windowSize.y);
    gl::setDefaultShaderVars();
    gl::drawSolidRect(rect);
    */
    // gl::clear(ColorAf(linearProgress,linearProgress,linearProgress,1));
    //mContent->render(offset);
    static_pointer_cast<SampleRenderable>(mContent)->render(offset, alpha);

    ci::gl::disable( GL_SCISSOR_TEST );
}

CINDER_APP_NATIVE( ConvergenceTestApp, RendererGl )
