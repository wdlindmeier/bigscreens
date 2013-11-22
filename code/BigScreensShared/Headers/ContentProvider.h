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
#include "DumbTank.h"

namespace bigscreens {
    
typedef std::shared_ptr<class SceneContentProvider> SceneContentProviderRef;
    
class SceneContentProvider
{
    
public:
    
    virtual ~SceneContentProvider(){}
    
    // Passes back a pointer to some renderable content.
    virtual RenderableContentRef contentForKey(const std::string & contentName) = 0;
    virtual float * getFFTData() = 0;
    virtual float getFFTDataForChannel(const int channel) = 0;
    
    // This is a crazy getter / setter combo.
    // I tried defining the shared pointer outside of the accessor,
    // but setting the value in a setter didn't carry over into the
    // getter... Works for now, but it would be good to figure out
    // the issue.
    static SceneContentProviderRef sharedContentProvider(SceneContentProvider *contentProvider = NULL)
	{
        static SceneContentProviderRef SharedContentProvider;
        if(!SharedContentProvider && contentProvider != NULL)
        {
            SharedContentProvider = SceneContentProviderRef(contentProvider);
        }
        return SharedContentProvider;
	}
};


namespace TextContentProvider {
    
    struct TextTimelineAndHeight
    {
        TextTimeline timeline;
        float absoluteLineHeight;
    };
    
    static TextTimelineAndHeight textTimelineForContentKey(const std::string & contentKey)
    {
        std::vector<TextWithFramecount> textTimeline;
        float lineHeight = 0;
        if (contentKey == "textRand")
        {
            lineHeight = 40;
            textTimeline.push_back(TextWithFramecount("LAUNCH\nCODE ]", 30));
            textTimeline.push_back(TextWithFramecount("LAUNCH\nCODE", 30));
        }
        else if (contentKey == "text0")
        {
            lineHeight = 40;
            textTimeline.push_back(TextWithFramecount("ATTACK\nSEQUENCE\nDELTA", 10000));
        }
        TextTimelineAndHeight ret;
        ret.timeline = textTimeline;
        ret.absoluteLineHeight = lineHeight;
        return ret;
    }
}
    
namespace ActorContentProvider {
	
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

}