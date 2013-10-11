#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"

#include "SceneWindow.h"
#include "ExampleContent.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class WindowImplApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void prepareSettings( Settings * settings ) { }
	void update();
	void draw();
	
	bigscreens::ExampleContent	mContent;
	gl::FboRef					mScratchFbo;
	gl::FboRef					mAccumulationFbo;
	int i;
};

void WindowImplApp::setup()
{
	mScratchFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight() );
	mAccumulationFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight() );
	i = 0;
	cout << "size " << sizeof( mContent ) << endl;
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
	bigscreens::OriginAndDimension mWindowDim( Vec2i( 0, 0 ), Vec2i( 200, 200 ) );
	// clear out the window with black
	{
		bigscreens::SceneWindowRef	mWindow( new bigscreens::SceneWindow( &mContent, &mWindowDim, mScratchFbo, mAccumulationFbo, ColorA::white() ) );
	}
	bigscreens::OriginAndDimension mNextWindowDim( Vec2i( 200, 200 ), Vec2i( 600, 400 ) );
	{
		bigscreens::SceneWindowRef  mNextWindow( new bigscreens::SceneWindow( &mContent, &mNextWindowDim, mScratchFbo, mAccumulationFbo, ColorA( 1.0f, 0.0f, 0.0f, 1.0f ) ) );
	}
	bigscreens::OriginAndDimension mThirdWindowDim( Vec2i( 200, 0 ), Vec2i( 400, 200 ) );
	{
		bigscreens::SceneWindowRef  mNextWindow( new bigscreens::SceneWindow( &mContent, &mThirdWindowDim, mScratchFbo, mAccumulationFbo, ColorA( 0.0f, 1.0f, 0.0f, 1.0f ) ) );
	}
	bigscreens::OriginAndDimension mFourthWindowDim( Vec2i( 0, 200 ), Vec2i( 200, 400 ) );
	{
		bigscreens::SceneWindowRef  mNextWindow( new bigscreens::SceneWindow( &mContent, &mFourthWindowDim, mScratchFbo, mAccumulationFbo, ColorA( 0.0f, 0.0f, 1.0f, 1.0f ) ) );
	}
	
	gl::viewport( Vec2i( 0, 0 ), Vec2i(640, 480) );
	gl::pushMatrices();
	gl::setMatricesWindow(getWindowSize());
	gl::draw( mScratchFbo->getTexture(), Vec2i( i, i ) );
	gl::popMatrices();
	
}

CINDER_APP_NATIVE( WindowImplApp, RendererGl )
