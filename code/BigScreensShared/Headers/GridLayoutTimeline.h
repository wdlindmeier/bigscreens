//
//  GridLayoutTimeline.h
//  GridWindowRefactor
//
//  Created by Ryan Bartley on 10/14/13.
//
//

#pragma once

#include "GridLayout.h"
#include "Utilities.hpp"

namespace bigscreens {
	
typedef std::shared_ptr<class GridLayoutTimeline> GridLayoutTimelineRef;
	
	// This is the container class for all GridLayouts
	// it will be where all transitional and current
	// GridLayout math will be constructed
	
class GridLayoutTimeline {
public:
	GridLayoutTimeline( const std::string & filePath = "", float scale = 1.0f )
	: mGridLayouts( GridLayout::loadAllFromPath( ( (filePath == "") ? bigscreens::SharedGridPath() : filePath ), scale ) ),
		currentLayoutIdx( 0 ), currentGridLayout( nullptr ), nextGridLayout( nullptr )
	{
	}
	
	~GridLayoutTimeline() {}
	
	const std::vector<GridLayout> & getGridLayouts() const { return mGridLayouts; }
	// This will be the thread safe update function that will create correct Screen regions
	// which will be used in the SceneWindow.
	std::pair<std::shared_ptr<GridLayout>, std::shared_ptr<GridLayout> > tick();
	
private:
	std::vector<GridLayout>		mGridLayouts;
	int							currentLayoutIdx, prevLayoutIdx;
	std::shared_ptr<GridLayout>	currentGridLayout, nextGridLayout;
};
	
}
