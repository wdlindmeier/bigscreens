//
//  Grid.h
//  GridMaker
//
//  Created by William Lindmeier on 10/8/13.
//
//

#pragma once

#include "SharedTypes.hpp"
#include "cinder/Cinder.h"

namespace bigscreens {
	
	const static long kDefaultTransitionDuration = 500;
	const static long kDefaultTimestampOffset = 2000;
	
	typedef enum GridRenderModes
	{
		GridRenderModeNormal = 0,
		GridRenderModeAdding,
		GridRenderModeRemoving,
		GridRenderModeJoining
	} GridRenderMode;
	
	class GridLayout {
	public:
		GridLayout();
		~GridLayout() {};
		
		static GridLayout load( const ci::fs::path & filePath, float scale = 1.0f );
		static std::vector<GridLayout> loadAllFromPath( const ci::fs::path & directory, float scale = 1.0f );
		
		void serialize( const ci::fs::path & directory, float scale = 1.0f );
		void remove();
		
		std::string getName() const { return mName; }
		void setName( const std::string & name ) { mName = name; }
		
		// turn this into a vector of Rects
		std::vector<bigscreens::ScreenRegion> getRegions() const { return mRegions; }
		void setRegions( const std::vector<bigscreens::ScreenRegion> & regions ) { mRegions = regions; }
		void addRegion( const bigscreens::ScreenRegion & region ) { mRegions.push_back(region); }
		
		long long getTimestamp() const { return mTimestamp; }
		void setTimestamp( const long long & timestamp ) { mTimestamp = timestamp; }
		
		long getTransitionDuration() const { return mTransitionMillisec; }
		void setTransitionDuration(const long durationMillisec){ mTransitionMillisec = durationMillisec; }
		
		std::string getUniqueID(){ return mUniqueID; };
		void setUniqueID( const std::string & uid ) { mUniqueID = uid; }
		
		bool operator==(GridLayout & other){ return mUniqueID == other.getUniqueID(); };
		
		void setPath(const ci::fs::path & path){ mPath = path; };
		ci::fs::path getPath(){ return mPath; }
		
	private:
		std::vector<bigscreens::ScreenRegion> mRegions;
		std::string mName;
		std::string mUniqueID;
		long long mTimestamp;
		long mTransitionMillisec;
		ci::fs::path mPath;
	};
	
}