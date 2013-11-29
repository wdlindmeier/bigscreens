//
//  TankRevealOpponent.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/29/13.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "TankContent.h"
#include "SharedTypes.hpp"
#include "ContentProvider.h"
#include "cinder/Easing.h"

namespace bigscreens
{
    
class TankRevealOpponentContent : public TankContent
{
    
public:
    
    TankRevealOpponentContent();
    ~TankRevealOpponentContent(){};
    
    // void render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect);
    // Override draw minion to draw the opponent
    void update(std::function<void (ci::CameraPersp & cam, AdvancedTankRef & tank)> update_func);
    void drawMinion();
    void renderOpponent();

private:

    OpponentRef mOpponent;
    
};
    
}