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
        if (contentKey == "textLaunchCode")
        {
            lineHeight = 40;
            textTimeline.push_back(TextWithFramecount("LAUNCH\nCODE ]", 30));
            textTimeline.push_back(TextWithFramecount("LAUNCH\nCODE", 30));
        }
        else if (contentKey == "textSeqDelta")
        {
            lineHeight = 40;
            textTimeline.push_back(TextWithFramecount("ATTACK\nSEQUENCE\nDELTA", 10000));
        }
        else if (contentKey == "textTargetEngaged")
        {
            lineHeight = 40;
            textTimeline.push_back(TextWithFramecount("TARGET ENGAGED", 10000));
        }
        else if (contentKey == "textTargetAcquired")
        {
            lineHeight = 40;
            textTimeline.push_back(TextWithFramecount("TARGET ACQUIRED", 10000));
        }
        else if (contentKey == "textAssistanceRequested")
        {
            lineHeight = 40;
            textTimeline.push_back(TextWithFramecount("ASSISTANCE REQUESTED", 10000));
        }
        if (contentKey == "textCounter")
        {
            lineHeight = 40;
            textTimeline.push_back(TextWithFramecount("95:920", 40));
            textTimeline.push_back(TextWithFramecount("95:921", 40));
            textTimeline.push_back(TextWithFramecount("95:922", 40));
            textTimeline.push_back(TextWithFramecount("95:923", 40));
            textTimeline.push_back(TextWithFramecount("95:924", 40));
            textTimeline.push_back(TextWithFramecount("95:925", 40));
            textTimeline.push_back(TextWithFramecount("95:926", 40));
            textTimeline.push_back(TextWithFramecount("95:927", 40));
            textTimeline.push_back(TextWithFramecount("95:928", 40));
            textTimeline.push_back(TextWithFramecount("95:929", 40));
        }
        if (contentKey == "textComInterrupt")
        {
            lineHeight = 40;
            textTimeline.push_back(TextWithFramecount("COM INTERRUPT ]", 30));
            textTimeline.push_back(TextWithFramecount("COM INTERRUPT", 30));
        }
        if (contentKey == "textID0")
        {
            lineHeight = 20;
            textTimeline.push_back(TextWithFramecount("VEHICLE:\n1103284", 30));
        }
        if (contentKey == "textID1")
        {
            lineHeight = 20;
            textTimeline.push_back(TextWithFramecount("VEHICLE:\n1107288", 30));
        }
        if (contentKey == "textID2")
        {
            lineHeight = 20;
            textTimeline.push_back(TextWithFramecount("VEHICLE:\n1000315", 30));
        }
        if (contentKey == "textAss")
        {
            lineHeight = 100;
            textTimeline.push_back(TextWithFramecount("*", 30));
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