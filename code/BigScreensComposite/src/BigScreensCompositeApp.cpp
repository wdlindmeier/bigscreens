#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "MPEApp.hpp"
#include "MPEClient.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace mpe;

class BigScreensCompositeApp : public AppNative, public MPEApp
{
    
public:
    
    void prepareSettings(Settings *settings);
	void setup();
    ci::DataSourceRef mpeSettingsFile();
    void mpeReset();
    
	void update();
    void mpeFrameUpdate(long serverFrameNumber);
	
    void draw();
    void mpeFrameRender(bool isNewFrame);    
    
    MPEClientRef mClient;
};

void BigScreensCompositeApp::prepareSettings(Settings *settings)
{
    settings->setBorderless();
}

void BigScreensCompositeApp::setup()
{
    mClient = MPEClient::Create(this);
}

ci::DataSourceRef BigScreensCompositeApp::mpeSettingsFile()
{
    return ci::app::loadResource("settings."+to_string(CLIENT_ID)+".xml");
}

void BigScreensCompositeApp::mpeReset()
{
    // Reset the state of your app.
    // This will be called when any client connects.
}

void BigScreensCompositeApp::update()
{
    if (!mClient->isConnected() && (getElapsedFrames() % 60) == 0)
    {
        mClient->start();
    }
}

void BigScreensCompositeApp::mpeFrameUpdate(long serverFrameNumber)
{
    // Your update code.
}

void BigScreensCompositeApp::draw()
{
    mClient->draw();
}

void BigScreensCompositeApp::mpeFrameRender(bool isNewFrame)
{
    gl::clear(Color(0.5,1.0,0.5));
    // Your render code.
}

// If you're deploying to iOS, set the Render antialiasing to 0 for a significant
// performance improvement. This value defaults to 4 (AA_MSAA_4) on iOS and 16 (AA_MSAA_16)
// on the Desktop.
#if defined( CINDER_COCOA_TOUCH )
CINDER_APP_NATIVE( BigScreensCompositeApp, RendererGl(RendererGl::AA_NONE) )
#else
CINDER_APP_NATIVE( BigScreensCompositeApp, RendererGl )
#endif
