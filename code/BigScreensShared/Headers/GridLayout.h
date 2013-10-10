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
#include "cinder/gl/Texture.h"


namespace bigscreens
{

const static long kDefaultTransitionDuration = 500;
const static long kDefaultTimestampOffset = 2000;

typedef enum GridRenderModes
{
    GridRenderModeNormal = 0,
    GridRenderModeAdding,
    GridRenderModeRemoving,
    GridRenderModeJoining
} GridRenderMode;
    
class GridLayout
{
    
public:
    
    GridLayout();
    //GridLayout(const GridLayout & gl);
    ~GridLayout(){};

    static GridLayout load(const cinder::fs::path & filePath);
    static std::vector<GridLayout> loadAllFromPath(const cinder::fs::path & directory);
    
    void loadAssets();
    void serialize();
    void remove();
    
    std::string getName() const { return mName; };
    void setName(const std::string & name){ mName = name; };

    std::vector<bigscreens::ScreenRegion> getRegions() const { return mRegions; };
    void setRegions(const std::vector<bigscreens::ScreenRegion> & regions){ mRegions = regions; };
    void addRegion(const bigscreens::ScreenRegion & region){ mRegions.push_back(region); };
    
//    void update();
    void render(const float transitionAmount,
                const GridLayout & otherLayout,
                const ci::Vec2f & mousePosition,
                const GridRenderMode mode,
                bigscreens::RenderableContent & content);
    
    long long getTimestamp() const { return mTimestamp; };
    void setTimestamp(const long long timestamp){ mTimestamp = timestamp; }

    long getTransitionDuration() const { return mTransitionMillisec; };
    void setTransitionDuration(const long durationMillisec){ mTransitionMillisec = durationMillisec; }

private:
    
    std::vector<bigscreens::ScreenRegion> mRegions;
    std::string mName;
    ci::gl::Texture mScreenTexture;
    long long mTimestamp;
    long mTransitionMillisec;
    
};
    
}