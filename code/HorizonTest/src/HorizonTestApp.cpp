#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class HorizonTestApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void HorizonTestApp::setup()
{
}

void HorizonTestApp::mouseDown( MouseEvent event )
{
}

void HorizonTestApp::update()
{
}

void HorizonTestApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( HorizonTestApp, RendererGl )
