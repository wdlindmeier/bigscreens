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
#include "GroundContent.h"
#include "DumbTank.h"

namespace bigscreens {

namespace ContentProviderNew {

namespace ActorContent {
	
	static AdvancedTankRef getAdvancedTank()
	{
		static AdvancedTankRef masterAdvancedTank;
		if( !masterAdvancedTank )
        {
            ci::app::console() << "Initting Advanced Tank\n";
			masterAdvancedTank = AdvancedTankRef( new AdvancedTank() );
		}
        return masterAdvancedTank;
	}
	
	static FinalBillboardRef getFinalBillboard()
	{
		static FinalBillboardRef masterFinalBillboard;
		if( !masterFinalBillboard )
        {
            ci::app::console() << "Initting Final Billboard\n";
			masterFinalBillboard = FinalBillboardRef( new FinalBillboard() );
		}
        return masterFinalBillboard;
	}
	
	static FloorPlaneRef getFloorPlane()
	{
		static FloorPlaneRef masterFloorPlane;
		if( !masterFloorPlane )
        {
            ci::app::console() << "Initting Ground Plane\n";
			masterFloorPlane = FloorPlaneRef( new FloorPlane() );
		}
        return masterFloorPlane;
	}
	
	static OpponentRef getOpponent()
	{
		static OpponentRef masterOpponent;
		if( !masterOpponent )
        {
            ci::app::console() << "Initting Opponent\n";
			masterOpponent = OpponentRef( new Opponent() );
		}
        return masterOpponent;
	}
	
	static MinionRef getMinion()
	{
		static MinionRef masterMinion;
		if( !masterMinion )
        {
            ci::app::console() << "Initting Minion\n";
			masterMinion = MinionRef( new PyramidalGeometry() );
		}
        return masterMinion;
	}

    static GroundContentRef getGroundContent()
	{
		static GroundContentRef masterGround;
		if( !masterGround )
        {
            ci::app::console() << "Initting Ground Content\n";
			masterGround = GroundContentRef( new GroundContent(10000) );
		}
        return masterGround;
	}
    
    static DumbTankRef getDumbTank()
	{
		static DumbTankRef masterDumbTank;
		if( !masterDumbTank )
        {
            ci::app::console() << "Initting Dumb Tank Content\n";
			masterDumbTank = DumbTankRef( new DumbTank() );
		}
        return masterDumbTank;
	}
}
	
namespace SceneContent {
	
	static TankContentRef getTankContent()
	{
		static TankContentRef masterTankContent;
		if( !masterTankContent )
        {
            ci::app::console() << "Initting Tank Content Ref\n";
			masterTankContent = TankContentRef( new TankContent() );
		}
        return masterTankContent;
	}
	
	
}
	
}

}