//
//  Grid.cpp
//  GridMaker
//
//  Created by William Lindmeier on 10/8/13.
//
//

#include "GridLayout.h"
#include "cinder/Utilities.h"
#include "Utilities.hpp"
#include <fstream>
#include <boost/iterator/filter_iterator.hpp>

using namespace std;
using namespace cinder;

namespace bigscreens {
	
GridLayout::GridLayout()
: mName(""), mRegions(), mTimestamp(0),
mTransitionMillisec(1000),
mUniqueID( to_string( arc4random() % 99999999 ) )
{
}

// this is a highly optimized copy constructor for the timeline class
// all we're really interested in is grabbing the regions so we can
// manipulate them
GridLayout::GridLayout( const GridLayout * other )
{
	mRegions = other->mRegions;
}

GridLayout GridLayout::load( const ci::fs::path &filePath, float scale )
{
	GridLayout layout;
	vector<ScreenRegion> regions;
	if( fs::exists( filePath ) )
	{
		fstream inFile( filePath.string() );
		string line;
		int lineCount = 0;
		while( getline( inFile, line ) )
		{
			vector<string> tokens = ci::split( line, "," );
			
			if( tokens.size() ) {
				if( lineCount == 0 ) {
					if( tokens.size() > 1 ) {
						layout.setTimestamp( stoll( tokens[0] ) );
						layout.setTransitionDuration( stoll( tokens[1] ) );
					}
				}
				else {
					int x1 = stoi( tokens[0] ) * scale;
					int y1 = stoi( tokens[1] ) * scale;
					int x2 = stoi( tokens[2] ) * scale;
					int y2 = stoi( tokens[3] ) * scale;
					ScreenRegion reg( x1, y1, x2, y2 );
					reg.isActive = true;
					regions.push_back( reg );
				}
			}
			lineCount++;
		}
		
		layout.setName( filePath.filename().string() );
		layout.setPath( filePath );
		layout.setUniqueID( layout.getName() );
		layout.setRegions( regions );
	}
	else {
		cout << "ERROR: File doesn't exist at path " << filePath << endl;
	}
	return layout;
}

void GridLayout::serialize( const ci::fs::path &directory, float scale )
{
	if( mName == "" ) {
		// Just randomizing the name. This could (very rarely) clobber another file.
		// Exciting!!!
		mName = mUniqueID;
	}
	mPath = directory / mName;
	
	ofstream oStream( mPath.string() );
	
	vector<string> output;
	
	oStream << to_string( mTimestamp ) << ", " << to_string( mTransitionMillisec ) << "\n";
	
	for( int i = 0; i < mRegions.size(); ++i ) {
		ScreenRegion & reg = mRegions[i];
		if( reg.isActive ) {
			oStream << to_string( (int)( reg.rect.x1 / scale ) ) << ",";
			oStream << to_string( (int)( reg.rect.y1 / scale ) ) << ",";
			oStream << to_string( (int)( reg.rect.x2 / scale ) ) << ",";
			oStream << to_string( (int)( reg.rect.y2 / scale ) ) << ",";
			oStream << "\n";
		}
	}
	oStream.close();
}

vector<GridLayout> GridLayout::loadAllFromPath( const ci::fs::path &directory, float scale )
{
	fs::directory_iterator dir_first( directory ), dir_last;
	
	auto pred = []( const fs::directory_entry &p )
	{
		string filename( p.path().filename().c_str() );
		transform( filename.begin(), filename.end(), filename.begin(), ::tolower );
		return fs::is_regular_file( p ) && filename.find( ".grid" ) != -1;
	};
	
	vector<fs::path> gridFiles;
	copy( make_filter_iterator( pred, dir_first, dir_last ),
		  make_filter_iterator( pred, dir_last,  dir_last ),
		  back_inserter( gridFiles ) );
	
	vector<GridLayout> gridLayouts;
	
	for( auto gridFileIt = gridFiles.begin(); gridFileIt != gridFiles.end(); ++gridFileIt ) {
		GridLayout gridLayout = GridLayout::load( *gridFileIt, scale );
		
		if( gridLayout.getTimestamp() == 0 ) {
			cout << "Adding a default timestamp\n";
			
			gridLayout.setTimestamp( kDefaultTimestampOffset * ( gridFileIt - gridFiles.begin() ) );
			gridLayout.setTransitionDuration( kDefaultTransitionDuration );
		}
		if( gridLayout.getRegions().size() > 0 )
			gridLayouts.push_back( gridLayout );
	}
	
	sort( gridLayouts.begin(), gridLayouts.end(), sortByTimestamp );
	
	return gridLayouts;
}

void GridLayout::remove()
{
	if( mPath != fs::path() )
		fs::remove( mPath );
}
}