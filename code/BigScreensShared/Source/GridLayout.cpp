//
//  Grid.cpp
//  GridMaker
//
//  Created by William Lindmeier on 10/8/13.
//
//

#include "GridLayout.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"
#include <fstream>

using namespace cinder;
using namespace std;

namespace bigscreens
{
    
GridLayout GridLayout::load(const fs::path & filePath)
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
            lineCount++;
            vector<string> tokens = ci::split(line, ",");
            int x1 = stoi(tokens[0]);
            int y1 = stoi(tokens[1]);
            int x2 = stoi(tokens[2]);
            int y2 = stoi(tokens[3]);
            ScreenRegion reg(x1,y1,x2,y2);
            reg.isActive = true;
            regions.push_back(reg);
        }
        
        layout.setName(filePath.filename().string());
        layout.setRegions(regions);
    }
    else
    {
        ci::app::console() << "ERROR: File doesn't exist at path " << filePath << endl;
    }
    return layout;
}
    
    
void GridLayout::serialize()
{
    string filename = mName;
    if (filename == "")
    {
        time_t t;
        long timestamp = time(&t);
        filename = to_string(timestamp) + ".grid";
    }
    fs::path gridPath = cinder::app::getAssetPath(".") / filename;
    
    std::ofstream oStream( gridPath.string() );
    
    vector<string> output;
    for (int i = 0; i < mRegions.size(); ++i)
    {
        ScreenRegion & reg = mRegions[i];
        if (reg.isActive)
        {
            oStream << to_string((int)reg.rect.x1) << ",";
            oStream << to_string((int)reg.rect.y1) << ",";
            oStream << to_string((int)reg.rect.x2) << ",";
            oStream << to_string((int)reg.rect.y2) << ",";
            oStream << "\n";
        }
    }
    oStream.close();
}

void GridLayout::remove()
{
    if (mName != "")
    {
        fs::path gridPath = ci::app::getAssetPath(mName);
        fs::remove(gridPath);
    }
}

}