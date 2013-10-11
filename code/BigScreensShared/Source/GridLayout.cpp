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
#include "Utilities.hpp"
#include <fstream>
#include <boost/iterator/filter_iterator.hpp>

using namespace cinder;
using namespace std;

// Shared screen texture.
// This should be part of the content.
static ci::gl::Texture ScreenTexture;

namespace bigscreens
{

GridLayout::GridLayout() :
    mName(""),
    mRegions(),
    mTimestamp(0),
    mTransitionMillisec(1000),
    mUniqueID(to_string(arc4random() % 9999999))
{
    loadAssets();
};

void GridLayout::loadAssets()
{
    if (!ScreenTexture)
    {
        ScreenTexture = loadImage(app::loadResource("screen.png"));
    }
}
    
GridLayout GridLayout::load(const fs::path & filePath, float scale)
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
                    ScreenRegion reg(x1,y1,x2,y2);
                    reg.isActive = true;
                    regions.push_back(reg);
                }
            }
            lineCount++;
        }
        
        layout.setName(filePath.filename().string());
        layout.setUniqueID(layout.getName());
        layout.setRegions(regions);
    }
    else
    {
        ci::app::console() << "ERROR: File doesn't exist at path " << filePath << endl;
    }
    return layout;
}
    
void GridLayout::serialize(const cinder::fs::path & directory, float scale)
{
    if (mName == "")
    {
        // Just randomizing the name. This could (very rarely) clobber another file.
        // Exciting!!!
        mName = mUniqueID;
    }
    mPath = directory / mName;
    
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
            oStream << to_string((int)(reg.rect.x1 / scale)) << ",";
            oStream << to_string((int)(reg.rect.y1 / scale)) << ",";
            oStream << to_string((int)(reg.rect.x2 / scale)) << ",";
            oStream << to_string((int)(reg.rect.y2 / scale)) << ",";
            oStream << "\n";
        }
    }
    oStream.close();
}
 
std::vector<GridLayout> GridLayout::loadAllFromPath(const cinder::fs::path & directory, float scale)
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
        GridLayout gridLayout = GridLayout::load(gridPath, scale);
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
    std::sort(gridLayouts.begin(), gridLayouts.end(), sortByTimestamp);
    
    return gridLayouts;
}

void GridLayout::remove()
{
    if (mPath != fs::path())
    {
        fs::remove(mPath);
    }
}
    
/*
void GridLayout::update()
{
//    mContent.update();
}
*/
    
void GridLayout::render(const float transitionAmount,
                        const GridLayout & otherLayout,
                        const Vec2f & mousePosition,
                        const GridRenderMode mode,
                        RenderableContent & content)
{
    int regionSize = mRegions.size();
    
    vector<ScreenRegion> compareRegions = otherLayout.getRegions();
    int compareRegionSize = compareRegions.size();
    
    for (int i = 0; i < regionSize; ++i)
    {
        ScreenRegion & reg = mRegions[i];
        
        bool isHovered = reg.rect.contains(mousePosition);
        bool isJoining = reg.isSelected || ((mode == GridRenderModeJoining) && isHovered);
        bool isRemoving = ((mode == GridRenderModeRemoving) && isHovered);
        
        float alpha = transitionAmount;
        
        Rectf rA = reg.rect;
        
        if(reg.isActive)
        {
            if (transitionAmount < 1.0f)
            {
                // Check if this is a persistant region
                for (int j = 0; j < compareRegionSize; ++j)
                {
                    ScreenRegion compareReg = compareRegions[j];
                    
                    Rectf rB = compareReg.rect;
                    
                    if (compareReg.isActive && rectCompare(rA, rB))
                    {
                        // No transition. Just draw at full blast.
                        alpha = 1.0f;
                        break;
                    }
                }
            }
            
            if (isJoining)
            {
                ScreenTexture.unbind();
                gl::color(ColorAf(1.0f,1.0f,0.0f,alpha));
            }
            else if (isRemoving)
            {
                ScreenTexture.unbind();
                gl::color(ColorAf(1.0f,0.0f,0.0f,alpha));
            }
            else
            {
                ColorAf transColor = reg.color;
                transColor[3] = alpha;
                gl::color(transColor);
                
                if (ScreenTexture)
                {
                    ScreenTexture.enableAndBind();
                }
            }
            
            gl::pushMatrices();
            gl::translate(rA.getCenter());
            gl::scale(alpha, alpha, 1.0f);
            
            Rectf scaledRect(rA.getWidth() * -0.5f,
                             rA.getHeight() * -0.5f,
                             rA.getWidth() * 0.5f,
                             rA.getHeight() * 0.5f);
            gl::drawSolidRect(scaledRect);
            
            gl::Texture contentTexture = content.getTexture();
            Rectf tankBounds = contentTexture.getBounds();
            
            // "Get cetered fill"
            float aspectWidth = scaledRect.getWidth() / tankBounds.getWidth();
            float aspectHeight = scaledRect.getHeight() / tankBounds.getHeight();
            float scaleFactor = std::max<float>(aspectWidth, aspectHeight);
            Vec2f drawSize = scaledRect.getSize();
            float areaMarginX = drawSize.x > drawSize.y ? 0 : ((drawSize.y - drawSize.x) * 0.5f);
            float areaMarginY = drawSize.x > drawSize.y ? ((drawSize.x - drawSize.y) * 0.5f) : 0;
            areaMarginX /= scaleFactor;
            areaMarginY /= scaleFactor;
            float areaWidth = scaledRect.getWidth() / scaleFactor;
            float areaHeight = scaledRect.getHeight() / scaleFactor;
            Area drawArea(areaMarginX,
                          areaMarginY,
                          areaMarginX + areaWidth,
                          areaMarginY + areaHeight);
            
            gl::draw(contentTexture,
                     drawArea,
                     scaledRect);
            
            if (ScreenTexture)
            {
                ScreenTexture.unbind();
            }
            
            gl::lineWidth(2.0f);
            gl::color(1.0f,1.0f,1.0f,std::min<float>(0.8f, alpha));
            gl::drawStrokedRect(scaledRect);            
            gl::popMatrices();
        }
        else
        {
            gl::lineWidth(1.0f);

            if (isJoining)
            {
                gl::color(ColorAf(1.0f,1.0f,0.0f,alpha));
            }
            else if (isRemoving)
            {
                gl::color(ColorAf(1.0f,0.0f,0.0f,alpha));
            }
            else
            {
                gl::color(ColorAf(0.25f, 0.25f, 0.25f, alpha));
            }
            gl::drawStrokedRect(rA);
        }
    }
}

}