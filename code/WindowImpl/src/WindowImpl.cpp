#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"

#include "Window.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class WindowImplApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	
	bigscreens::Content			mContent;
	gl::FboRef					mScratchFbo;
	int i;
};

void WindowImplApp::setup()
{
	mScratchFbo = gl::Fbo::create( 640, 480 );
	i = 0;
}

void WindowImplApp::mouseDown( MouseEvent event )
{
}

void WindowImplApp::update()
{
	i++;
}

void WindowImplApp::draw()
{
	gl::clear();
	bigscreens::OriginAndDimension mWindowDim( Vec2i( 0, 0 ), Vec2i( 100, 100 ) );
	// clear out the window with black
	{
		bigscreens::SceneWindowRef	mWindow( new bigscreens::SceneWindow( &mContent, &mWindowDim, mScratchFbo, ColorA::white()) );
	}
	bigscreens::OriginAndDimension mNextWindowDim( Vec2i( 100, 0 ), Vec2i( 320, 240 ) );
	{
		bigscreens::SceneWindowRef  mNextWindow( new bigscreens::SceneWindow( &mContent, &mNextWindowDim, mScratchFbo, ColorA(1.0f, 0.0f, 0.0f, 1.0f) ) );
	}

	gl::viewport( Vec2i( 0, 0 ), Vec2i(640, 480) );
	gl::pushMatrices();
	gl::setMatricesWindow(getWindowSize());
	gl::draw( mScratchFbo->getTexture(), Vec2i( i, i ) );
	gl::popMatrices();
	
}

CINDER_APP_NATIVE( WindowImplApp, RendererGl )
