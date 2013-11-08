//
//  SetupContent.hpp
//  BigScreensComposite
//
//  Created by Ryan Bartley on 11/5/13.
//
//

#pragma once 

// This will be where we load each Ref and then pass them to the specific places they need to be
// i.e.
//
// tank = Tank::create();
// groundPlane = GroundPlane::create();
// opponent = Opponent::create();
// convergence = Convergence::create( tank, groundPlane, opponent );
// traveling = Traveling::create( tank, groundPlane );
