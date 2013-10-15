#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "GridLayout.h"
#include "SceneWindow.hpp"
#include "Utilities.hpp"
#include "DummyContent.h"
#include "cinder/gl/Fbo.h"
#include "GridLayoutTimeline.h"
#include "TankContent.h"

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
	
	bigscreens::GridLayoutTimelineRef		mGridTimeline;
	std::vector<bigscreens::GridLayout>		mGridLayouts;
	bigscreens::ExampleContent				mExampleContent;
	bigscreens::TankContent					mTankContent;
	gl::FboRef								mScratchFbo;
	std::vector<bigscreens::ScreenRegion>	mRegions;
	int										curLayoutIdx;
	bigscreens::OutLineBorderRef			mOutLine;
};

void GridWindowRefactorApp::setup()
{
	curLayoutIdx = 0;
    fs::path gridPath = bigscreens::SharedGridPath();
    mGridLayouts = bigscreens::GridLayout::loadAllFromPath(gridPath, kScreenScale);
	mRegions = mGridLayouts[curLayoutIdx].getRegions();
	mScratchFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight() );
	
	mOutLine = std::shared_ptr<bigscreens::OutLineBorder>(new bigscreens::OutLineBorder());
	
	mTankContent.load( "../../BigScreensShared/Resources/T72.obj" );
	
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
	mTankContent.update();
	
	// clear out the window with black
	// we don't need to clear every frame only at the beginning and whenever we're transitioning
	gl::clear( Color( 0, 0, 0 ) );
	
	//	here or probably in update we'll tick() the timeline and get back our layouts
	//	auto gridLayouts = mGridTimeline->tick();
	//  once we get it worked out we'll actually send tick to another thread after rendering and get the std::pair result as a future
	//	before rendering.
	//	first check if we need to do anything with the next gridlayout
	//	if( !gridLayouts.second )
	//		and if there's not just draw the gridLayouts in .first->getRegions() like below
	for( auto dimIt = mRegions.begin(); dimIt != mRegions.end(); ++dimIt ) {
		bigscreens::SceneWindowRef  mNextWindow( new bigscreens::SceneWindow( &mTankContent, &(*dimIt), mScratchFbo, mOutLine )  );
	}
	//  else {
	//	We'll have to figure out the best way to transform. maybe scenewindow can take a color or alpha value like you were doing before and
	//	use that to smoothly transition I'm open to suggestions.
}

void GridWindowRefactorApp::draw()
{
	
}

CINDER_APP_NATIVE( GridWindowRefactorApp, RendererGl )
