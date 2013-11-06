#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "SharedTypes.hpp"
#include "Utilities.hpp"
#include "GridLayout.h"
#include "SampleRenderable.h"
#include "SceneWindow.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace bigscreens;

class ConvergenceTestApp : public AppNative {
  public:
    void prepareSettings(Settings *settings);
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    GridLayout mLayout;
    //SampleRenderable mContent;
    RenderableContentRef mContent;
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
}

void ConvergenceTestApp::mouseDown( MouseEvent event )
{
}

void ConvergenceTestApp::update()
{
}

void ConvergenceTestApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0.5, 0.5, 0.5 ) );
    
    Vec2i windowSize = getWindowSize();
    
    for (ScreenRegion region : mLayout.getRegions())
    {
        static_pointer_cast<SampleRenderable>(mContent)->update([=](CameraPersp & cam){
            // cam.setPerspective( 45.0f, getWindowAspectRatio(), .01, 40000 );
            float camX = 1000;//cosf(tankRotation) * 1000;
            float camZ = 1000;//sinf(tankRotation) * 1000;
            cam.lookAt( Vec3f( camX, 400, camZ ), Vec3f( 0, 100, 0 ) );
        });
        
        SceneWindow scene(mContent,
                          region.rect,
                          windowSize);
        
        scene.render(Vec2i::zero());
    }
    
    
    // NOTE:
    // How exactly does this work?
    // mOutLine->render();
}

CINDER_APP_NATIVE( ConvergenceTestApp, RendererGl )
