//
//  GridLayoutTimeline.cpp
//  GridWindowRefactor
//
//  Created by Ryan Bartley on 10/14/13.
//
//

#include "GridLayoutTimeline.h"

namespace bigscreens {

// disregard all current code in here
std::pair<std::shared_ptr<GridLayout>, std::shared_ptr<GridLayout> > GridLayoutTimeline::tick()
{
	
	// We can use an index to tell us which layout we're on
	// Figure out if this Layout will transition
	// if it is transitioning create * futureLayout (for example)
	// which is the next layout.
	// create a newed GridLayout pointer that can transition.
	// something like this.
	
	// increment the time and figure out if we're about to start transitioning.
	// if we're in a transition we'll go down one path otherwise we'll go down passing
	// the current Gridlayout by itself with a default constructed shared_ptr in the second case
	
	currentGridLayout = std::shared_ptr<GridLayout>( new GridLayout( &mGridLayouts[currentLayoutIdx] ));
	// we have to be careful about memory management
	// I'm implementing here with shared_ptr because of thread stuff but I wouldn't mind using pure pointers
	
//	if(this transitioning ) {
//		for(auto curRegionIt = currentGridLayout->getRegions().begin(); curRegionIt != currentGridLayout->getRegions().end(); ++curRegionIt ) {
			// iterate through each region to make it the correct size
//		}
		// if we're in or about to be in a transition create another
		// pointer for the future regions if currentLayoutIdx+1 is not greater than size
		nextGridLayout = std::shared_ptr<GridLayout>( new GridLayout( &mGridLayouts[currentLayoutIdx+1] ) );
		// manipulate any thing that needsto be manipulated about current and future
		// then pass along pointers to each std::pair<GridLayout*, GridLayout*>(CURRENTPOINTER, FUTUREPOINTER || 0)
		
//		return std::pair<GridLayout*, GridLayout*>(curLayout, nextGridLayout);
//	}
//	else
//		return std::pair<GridLayout*, GridLayout*>(curLayout, 0);
	// on the other side we'll check if it's a good pointer or not
	
	// also if we're finished with nextGridLayout and it is becoming currentGridLayout,
	// we just do currentGridLayout.reset( nextGridLayout.get() )
	// and it will swap and get rid of the dummy data
}

}