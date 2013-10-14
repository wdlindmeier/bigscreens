#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "GridLayout.h"
#include "SceneWindow.hpp"
#include "Utilities.hpp"
#include "DummyContent.h"
#include "cinder/gl/Fbo.h"

const static float kScreenScale = 0.25f;

using namespace ci;
using namespace ci::app;
using namespace std;

class GridWindowRefactorApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void keyUp( KeyEvent event );
	void prepareSettings( Settings * settings ) { settings->setWindowSize( 960, 270 ); }
	void update();
	void draw();
	
	std::vector<bigscreens::GridLayout>		mGridLayouts;
	bigscreens::ExampleContent				mContent;
	gl::FboRef								mScratchFbo;
	std::vector<bigscreens::ScreenRegion>	mRegions;
	int										curLayoutIdx;
};

void GridWindowRefactorApp::setup()
{
	curLayoutIdx = 0;
    fs::path gridPath = cinder::app::getAssetPath(".") / ".." / ".." / "BigScreensShared" / "Assets" / "grid";
    mGridLayouts = bigscreens::GridLayout::loadAllFromPath(gridPath, kScreenScale);
	mRegions = mGridLayouts[curLayoutIdx].getRegions();
	mScratchFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight() );
	
	gl::clear();
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void GridWindowRefactorApp::keyUp( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_RIGHT ) {
		curLayoutIdx++;
		if( curLayoutIdx > mGridLayouts.size() - 1 )
			curLayoutIdx = 0;
		mRegions = mGridLayouts[curLayoutIdx].getRegions();
	}
	else if( event.getCode() == KeyEvent::KEY_LEFT ) {
		curLayoutIdx--;
		if( curLayoutIdx < 0 )
			curLayoutIdx = mGridLayouts.size() - 1;
		mRegions = mGridLayouts[curLayoutIdx].getRegions();
	}
		
}

void GridWindowRefactorApp::mouseDown( MouseEvent event )
{
	
}

void GridWindowRefactorApp::update()
{
	
}

void GridWindowRefactorApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	
	for( auto dimIt = mRegions.begin(); dimIt != mRegions.end(); ++dimIt ) {
		bigscreens::SceneWindowRef  mNextWindow( new bigscreens::SceneWindow( &mContent, &(*dimIt), mScratchFbo ) );
	}
	
}

CINDER_APP_NATIVE( GridWindowRefactorApp, RendererGl )
