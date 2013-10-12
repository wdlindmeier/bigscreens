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
	void prepareSettings( Settings * settings ) { settings->setWindowSize( 960, 270 ); }
	void update();
	void draw();
	
	bigscreens::ExampleContent		mContent;
	gl::FboRef						mScratchFbo;
	gl::FboRef						mAccumulationFbo;
	std::vector<Rectf>				mDimensions;
	int i;
};

void WindowImplApp::setup()
{
	mScratchFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight() );
	mAccumulationFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight() );
	i = 0;
	mDimensions.push_back(Rectf( 726, 11, 875, 135 ));
	mDimensions.push_back(Rectf( 11, 11, 290, 260 ));
	mDimensions.push_back(Rectf( 301, 11, 575, 260 ));
	mDimensions.push_back(Rectf( 586, 11, 715, 135 ));
	mDimensions.push_back(Rectf( 586, 146, 950, 260 ));
	mDimensions.push_back(Rectf( 586, 146, 950, 260 ));
	gl::clear();
	gl::enableDepthRead();
	gl::enableDepthWrite();
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
	
	
	// clear out the window with black
	for( auto dimIt = mDimensions.begin(); dimIt != mDimensions.end(); ++dimIt ) {
		bigscreens::SceneWindowRef  mNextWindow( new bigscreens::SceneWindow( &mContent, &(*dimIt), mScratchFbo, mAccumulationFbo, ColorA( 1.0f, 0.0f, 1.0f, 1.0f ) ) );
	}
	
	
//	gl::viewport( Vec2i( 0, 0 ), getWindowSize() );
//	gl::pushMatrices();
//	gl::setMatricesWindow(getWindowSize());
//	gl::draw( mScratchFbo->getTexture(), Vec2i( i, i ) );
//	gl::popMatrices();
	
}

CINDER_APP_NATIVE( WindowImplApp, RendererGl )
