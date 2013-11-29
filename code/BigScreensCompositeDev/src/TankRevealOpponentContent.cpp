//
//  TankRevealOpponent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/29/13.
//
//

#include "TankRevealOpponentContent.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace bigscreens;

TankRevealOpponentContent::TankRevealOpponentContent() :
mOpponent(ActorContentProvider::getOpponent())
{
    mGroundScale = Vec3f(kDefaultGroundScaleX, 10, kDefaultGroundScaleZ);
    mMountainMagnitude = 0;
}

// Override draw minion to draw the opponent
void TankRevealOpponentContent::drawMinion()
{
    // mOpponent
    renderOpponent();
}

void TankRevealOpponentContent::renderOpponent()
{
    mOpponent->update(10, Vec3f(0,1,0));
    
    // Draw the opponent
    gl::pushMatrices();
    
    gl::setMatrices(mCam);
    gl::translate(Vec3f(0, kOpponentScale * 3, -10000));
    gl::scale(Vec3f(kOpponentScale, kOpponentScale, kOpponentScale));
    
    // Arbitrary
    Vec3f lightPos(sin(mNumFramesRendered * 0.1),
                   cos(mNumFramesRendered * 0.06666),
                   cos(mNumFramesRendered * 0.03333));
    
    float zDepth = mCam.getEyePoint().length();
    mOpponent->draw(zDepth, lightPos);
    gl::popMatrices();
}
