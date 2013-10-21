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

namespace bigscreens
{

GridLayout::GridLayout() :
    mRegions(),
    mTimestamp(0),
    mTransitionMillisec(1000),
    mUniqueID(to_string(arc4random() % 9999999)),
    mName("")//mUniqueID + ".grid")
{
};

GridLayout::GridLayout(const GridLayout & other) :
mRegions(other.getRegions()),
mTimestamp(other.getTimestamp()),
mTransitionMillisec(other.getTransitionDuration()),
mUniqueID(to_string(arc4random() % 9999999)),
mName(other.getName())
//mName(mUniqueID + ".grid")
{
};

GridLayout GridLayout::load(const fs::path & filePath,
                            float scale,
                            const cinder::Vec2i & wrapSize)
{
    GridLayout layout;
    vector<ScreenRegion> regions;
    if (fs::exists(filePath))
    {
        fstream inFile(filePath.string());
        string line;
        int lineCount = 0;

        while (getline (inFile, line))
        {
            vector<string> tokens = ci::split(line, ",");
            
            if (tokens.size() > 0) // ignore blank lines
            {
                if (lineCount == 0)
                {
                    if (tokens.size() > 1)
                    {
                        // First line is timing info
                        layout.setTimestamp(stoll(tokens[0]));
                        layout.setTransitionDuration(stoll(tokens[1]));
                    }
                }
                else
                {
                    int x1 = stoi(tokens[0]) * scale;
                    int y1 = stoi(tokens[1]) * scale;
                    int x2 = stoi(tokens[2]) * scale;
                    int y2 = stoi(tokens[3]) * scale;
                    
                    // WRAP
                    if (wrapSize != Vec2i::zero())
                    {
                        // NOTE: Scaling wrap because the scale has already been applied
                        // to the coords.
                        Vec2f scaledWrap = wrapSize * scale;

                        float width = x2 - x1;
                        float height = y2 - y1;
                        int row = x1 / (int)scaledWrap.x;
                        x1 = x1 % (int)scaledWrap.x;
                        y1 = (scaledWrap.y * row) + y1;
                        // NOTE: Regions cant split across rows.
                        // Their n2 values will always be on the same row as n1.
                        x2 = x1 + width;
                        y2 = y1 + height;
                    }

                    ScreenRegion reg(x1,y1,x2,y2);
                    reg.isActive = true;
                    if (tokens.size() > 4)
                    {
                        reg.contentKey = tokens[4];
                    }
                    regions.push_back(reg);
                }
            }
            lineCount++;
        }
        
        string filename = filePath.filename().string();
        layout.setName(filename);
        layout.setUniqueID(layout.getName());
        layout.setRegions(regions);
    }
    else
    {
        ci::app::console() << "ERROR: File doesn't exist at path " << filePath << endl;
    }
    return layout;
}
    
std::vector<GridLayout> GridLayout::loadAllFromPath(const cinder::fs::path & directory,
                                                    float scale,
                                                    const cinder::Vec2i & wrapSize)
{    
    fs::directory_iterator dir_first(directory), dir_last;

    // Filter on the .grid filetype
    auto pred = [](const fs::directory_entry& p)
    {
        string filename = string(p.path().filename().c_str());
        std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
        return fs::is_regular_file(p) && filename.find( ".grid" ) != -1;
    };
    std::vector<fs::path> gridFiles;
    std::copy(make_filter_iterator(pred, dir_first, dir_last),
              make_filter_iterator(pred, dir_last, dir_last),
              std::back_inserter(gridFiles));

    std::vector<GridLayout> gridLayouts;
    for (int i = 0; i < gridFiles.size(); ++i)
    {
        fs::path gridPath = gridFiles[i];
        GridLayout gridLayout = GridLayout::load(gridPath, scale, wrapSize);
        gridLayout.setPath(gridPath);
        if (gridLayout.getTimestamp() == 0)
        {
            ci::app::console() << "Adding a default timestamp\n";
            // Add some default time and duration
            gridLayout.setTimestamp(kDefaultTimestampOffset * i);
            gridLayout.setTransitionDuration(kDefaultTransitionDuration);
        }
        
        if (gridLayout.getRegions().size() > 0)
        {
            gridLayouts.push_back(gridLayout);
        }
    }
    
    // Sort them by their timestamp
    std::sort(gridLayouts.begin(), gridLayouts.end(), sortGridLayoutsByTimestamp);
    
    return gridLayouts;

}
void GridLayout::serialize(const cinder::fs::path & directory,
                           float scale,
                           const cinder::Vec2i & wrapSize)
{
    if (mName == "")
    {
        // Just randomizing the name. This could (very rarely) clobber another file.
        // Exciting!!!
        mName = mUniqueID + ".grid";
    }
    mPath = directory / mName;
    
    ci::app::console() << "Saving to path: " << mPath << endl;
    std::ofstream oStream( mPath.string() );
    
    vector<string> output;
    
    // First line is timing info
    oStream << to_string(mTimestamp) << "," << to_string(mTransitionMillisec) << "\n";
    
    // Following is the rect regions
    for (int i = 0; i < mRegions.size(); ++i)
    {
        ScreenRegion & reg = mRegions[i];
        if (reg.isActive)
        {
            int x1 = reg.rect.x1 / scale;
            int y1 = reg.rect.y1 / scale;
            int x2 = reg.rect.x2 / scale;
            int y2 = reg.rect.y2 / scale;
            
            // UNWRAP
            if (wrapSize != Vec2i::zero())
            {
                int row = y1 / (int)wrapSize.y;
                int offsetX = row * (int)wrapSize.x;
                int offsetY = (row * (int)wrapSize.y) * -1;
                x1 += offsetX;
                x2 += offsetX;
                y1 += offsetY;
                y2 += offsetY;
            }
            
            oStream << to_string(x1) << ",";
            oStream << to_string(y1) << ",";
            oStream << to_string(x2) << ",";
            oStream << to_string(y2) << ",";
            string contentName = reg.contentKey;
            std::replace( contentName.begin(), contentName.end(), ',', '%'); // replace all commas
            oStream << contentName;
            oStream << "\n";
        }
    }
    oStream.close();
}

void GridLayout::remove()
{
	if( mPath != fs::path() )
		fs::remove( mPath );
}

}