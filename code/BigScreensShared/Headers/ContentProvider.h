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
//#include "GroundContent.h"
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
			masterFloorPlane = FloorPlaneRef( new FloorPlane(ci::Vec2i(50,50)) );
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
	
	static MinionGeometryRef getMinion()
	{
		static MinionGeometryRef masterMinion;
		if( !masterMinion )
        {
            ci::app::console() << "Initting Minion\n";
			masterMinion = MinionGeometryRef( new MinionGeometry() );
		}
        return masterMinion;
	}

    static DumbTankRef getDumbTank()
	{
		static DumbTankRef masterDumbTank;
		if( !masterDumbTank )
        {
            ci::app::console() << "Initting Dumb Tank Content\n";
			masterDumbTank = DumbTankRef( new DumbTank("tank.obj", 0) );
		}
        return masterDumbTank;
	}
    
    static DumbTankRef getAngledDumbTank()
	{
		static DumbTankRef masterAngledDumbTank;
		if( !masterAngledDumbTank )
        {
            ci::app::console() << "Initting Angled Dumb Tank Content\n";
            const static float kDumbTankBarrelAngle = -17;//-15.69;
			masterAngledDumbTank = DumbTankRef( new DumbTank("tank_angled.obj", kDumbTankBarrelAngle) );
		}
        return masterAngledDumbTank;
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