//
//  ContentProvider.h
//  BigScreensComposite
//
//  Created by Ryan Bartley on 11/8/13.
//
//

#pragma once

#include "FinalBillboard.h"
#include "SharedTypes.hpp"
#include "AdvancedTank.h"
#include "FloorPlane.h"
#include "Opponent.h"
#include "TankContent.h"

namespace bigscreens {

namespace ContentProviderNew {

namespace ActorContent {
	
	static AdvancedTankRef getAdvancedTank()
	{
		static AdvancedTankRef masterAdvancedTank;
		if( masterAdvancedTank )
			return masterAdvancedTank;
		else {
			masterAdvancedTank = AdvancedTankRef( new AdvancedTank() );
			return masterAdvancedTank;
		}
	}
	
	static FinalBillboardRef getFinalBillboard()
	{
		static FinalBillboardRef masterFinalBillboard;
		if( masterFinalBillboard )
			return masterFinalBillboard;
		else {
			masterFinalBillboard = FinalBillboardRef( new FinalBillboard() );
			return masterFinalBillboard;
		}
	}
	
	static FloorPlaneRef getFloorPlane()
	{
		static FloorPlaneRef masterFloorPlane;
		if( masterFloorPlane )
			return masterFloorPlane;
		else {
			masterFloorPlane = FloorPlaneRef( new FloorPlane() );
			return masterFloorPlane;
		}
	}
	
	static OpponentRef getOpponent()
	{
		static OpponentRef masterOpponent;
		if( masterOpponent )
			return masterOpponent;
		else {
			masterOpponent = OpponentRef( new Opponent() );
			return masterOpponent;
		}
	}
	
	static MinionRef getMinion()
	{
		static MinionRef masterMinion;
		if( masterMinion )
			return masterMinion;
		else {
			masterMinion = MinionRef( new PyramidalGeometry() );
			return masterMinion;
		}
	}
	
}
	
namespace SceneContent {
	
	static TankContentRef getTankContent()
	{
		static TankContentRef masterTankContent;
		if( masterTankContent )
			return masterTankContent;
		else {
			masterTankContent = TankContentRef( new TankContent() );
			return masterTankContent;
		}
	}
	
	
}
	
}

}