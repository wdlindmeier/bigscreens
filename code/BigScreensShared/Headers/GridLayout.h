//
//  Grid.h
//  GridMaker
//
//  Created by William Lindmeier on 10/8/13.
//
//

#pragma once

#include "SharedTypes.hpp"

namespace bigscreens
{
    
class GridLayout
{
    
public:
    
    GridLayout() : mName(""), mRegions() {};
    GridLayout(const GridLayout & gl) : mName(gl.getName()), mRegions(gl.getRegions()) {};
    ~GridLayout(){};

    static GridLayout load(const cinder::fs::path & filePath);
    void serialize();
    void remove();
    
    std::string getName() const { return mName; };
    void setName(const std::string & name){ mName = name; };

    std::vector<bigscreens::ScreenRegion> getRegions() const { return mRegions; };
    void setRegions(const std::vector<bigscreens::ScreenRegion> & regions){ mRegions = regions; };
    void addRegion(const bigscreens::ScreenRegion & region){ mRegions.push_back(region); };

private:
    
    std::vector<bigscreens::ScreenRegion> mRegions;
    std::string mName;
    
};
    
}