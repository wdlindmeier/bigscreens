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
#include <boost/iterator/filter_iterator.hpp>

using namespace cinder;
using namespace std;

namespace bigscreens
{

GridLayout::GridLayout() :
    mName(""),
    mRegions()
{
    loadAssets();
};

GridLayout::GridLayout(const GridLayout & gl) :
    mName(gl.getName()),
    mRegions(gl.getRegions())
{
    loadAssets();
};
    
void GridLayout::loadAssets()
{
    mScreenTexture = loadImage(app::loadResource("screen.png"));
}
    
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
    
std::vector<GridLayout> GridLayout::loadAllFromPath(const cinder::fs::path & directory)
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
        GridLayout gridLayout = GridLayout::load(gridFiles[i]);
        if (gridLayout.getRegions().size() > 0)
        {
            gridLayouts.push_back(gridLayout);
        }
    }
    return gridLayouts;
}

void GridLayout::serialize()
{
    string filename = mName;
    if (filename == "")
    {
        int randNum = arc4random() % 9999999;
        filename = to_string(randNum) + ".grid";
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
    
void GridLayout::update()
{
//    mContent.update();
}
    
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
                    
                    if (compareReg.isActive && RectCompare(rA, rB))
                    {
                        // No transition. Just draw at full blast.
                        alpha = 1.0f;
                        break;
                    }
                }
            }
            
            if (isJoining)
            {
                mScreenTexture.unbind();
                gl::color(ColorAf(1.0f,1.0f,0.0f,alpha));
            }
            else if (isRemoving)
            {
                mScreenTexture.unbind();
                gl::color(ColorAf(1.0f,0.0f,0.0f,alpha));
            }
            else
            {
                ColorAf transColor = reg.color;
                transColor[3] = alpha;
                gl::color(transColor);
                
                mScreenTexture.enableAndBind();
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
            
            mScreenTexture.unbind();
            
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