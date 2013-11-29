//
//  TankRevealOpponent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/29/13.
//
//

#include "TankRevealOpponentContent.h"

using namespace bigscreens;

TankRevealOpponentContent::TankRevealOpponentContent() :
mOpponent(ActorContentProvider::getOpponent())
{
    
}

/*
void TankRevealOpponentContent::render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect)
{
    
}
*/

// Override draw minion to draw the opponent
void TankRevealOpponentContent::drawMinion()
{
    // mOpponent
}