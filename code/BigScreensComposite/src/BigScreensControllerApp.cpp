#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "MPEApp.hpp"
#include "MPEClient.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace mpe;

class BigScreensControllerApp : public AppNative, public MPEApp
{
public:
	void setup();
    ci::DataSourceRef mpeSettingsFile();
    void mpeReset();
    
	void update();
    void mpeFrameUpdate(long serverFrameNumber);
	
    void draw();
    void mpeFrameRender(bool isNewFrame);
    
    MPEClientRef mClient;
};

void BigScreensControllerApp::setup()
{
    mClient = MPEClient::Create(this);
}

ci::DataSourceRef BigScreensControllerApp::mpeSettingsFile()
{
    return ci::app::loadResource("settings."+to_string(CLIENT_ID)+".xml");
}

void BigScreensControllerApp::mpeReset()
{
    // Reset the state of your app.
    // This will be called when any client connects.
}

void BigScreensControllerApp::update()
{
    if (!mClient->isConnected() && (getElapsedFrames() % 60) == 0)
    {
        mClient->start();
    }
}

void BigScreensControllerApp::mpeFrameUpdate(long serverFrameNumber)
{
    // Your update code.
}

void BigScreensControllerApp::draw()
{
    mClient->draw();
}

void BigScreensControllerApp::mpeFrameRender(bool isNewFrame)
{
    gl::clear(Color(1.0,0.5,0.5));
    // Your render code.
}

// If you're deploying to iOS, set the Render antialiasing to 0 for a significant
// performance improvement. This value defaults to 4 (AA_MSAA_4) on iOS and 16 (AA_MSAA_16)
// on the Desktop.
#if defined( CINDER_COCOA_TOUCH )
CINDER_APP_NATIVE( BigScreensControllerApp, RendererGl(RendererGl::AA_NONE) )
#else
CINDER_APP_NATIVE( BigScreensControllerApp, RendererGl )
#endif
