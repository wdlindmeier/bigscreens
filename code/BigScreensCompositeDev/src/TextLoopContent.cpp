//
//  TextLoopContent.cpp
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/21/13.
//
//

#include "TextLoopContent.h"
#include "cinder/gl/Shader.h"
#include "cinder/Text.h"
#include "cinder/gl/Texture.h"
#include "cinder/Font.h"
#include "cinder/ip/Resize.h"
#include "cinder/Utilities.h"

using namespace std;
using namespace ci;
using namespace ci::gl;
using namespace ci::app;
using namespace bigscreens;

const static char kCharMap[] =
{
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X','Y','Z','1','2','3','4','5','6',
    '7','8','9','0','-','=','@','#',':','%','*','_','+',':','.',']',
    '$','>',' ' // space is always last
};

const static int kCharCount = 51;
const static int kCharsWide = 16;
const static int kCharPxTall = 201;
const static float kCharPxWide = 2048.0f / kCharsWide;

static int indexOf(char a)
{
    for( int i = 0; i < kCharCount; ++i)
    {
        if (kCharMap[i] == a)
        {
            return i;
        }
    }
    return -1;
}

static ci::Rectf regionForChar(char c)
{
    Rectf rect;
    int charIdx = indexOf(c);
    if (charIdx != -1)
    {
        int x = charIdx % kCharsWide;
        int y = charIdx / kCharsWide;
        rect.x1 = x * kCharPxWide;
        rect.x2 = rect.x1 + kCharPxWide;
        rect.y1 = y * kCharPxTall;
        rect.y2 = rect.y1 + kCharPxTall;
    }
    return rect;
}

namespace bigscreens
{    
    const static float kHyperspaceNativeLineHeight = 201.0f;
    
    TextLoopContent::TextLoopContent()
    {
        mFontSurf = loadImage(loadResource("hyperspace.png"));
        mCam = CameraOrtho(0, 1, 1, 0, 0, 100000);
    }
    
    void TextLoopContent::load()
    {
    }
    
    void TextLoopContent::update()
    {
        //
    }
    
    void TextLoopContent::newLayoutWasSet(const GridLayout & currentLayout)
    {
        return;
        // Prune any content not found in this layout
        //map<string, string> address_book;
        //for ( auto address_entry : address_book )
        //address_entry.first << " < " << address_entry.second
        vector<int> pruneTextureIDs;
        for (auto contentTexture : mTextures)
        {
            bool didFindContent = false;
            int texContentID = contentTexture.first;
            for (ScreenRegion & sr : currentLayout.getRegions())
            {
                if (texContentID == sr.timelineID)
                {
                    didFindContent = true;
                    break;
                }
            }
            if (!didFindContent)
            {
                pruneTextureIDs.push_back(texContentID);
                // Clear from the map
            }
        }
        for (int tID : pruneTextureIDs)
        {
            // Remove from map
            // console() << "ALERT: ERASING CONTENT FOR ID: " << tID << "\n";
            mTextures.erase(tID);
            mTextureDurations.erase(tID);
            mContentDurations.erase(tID);
        }
    }
    
    bool TextLoopContent::hasTextForContentID(const int contentID)
    {
        return mTextures.find(contentID) != mTextures.end();
    }
    
    void TextLoopContent::setTextForContentID(const TextTimeline & textWithTime,
                                              const int contentID,
                                              const float absoluteLineHeight)
    {
        // assert(mContentID > -1 && mContentID < 16000);
        
        if (!hasTextForContentID(contentID))
        {
            console() << "ALERT: CREATING NEW STRINGS FOR ID: " << contentID << "\n";
            float scale = (float)absoluteLineHeight / (float)kHyperspaceNativeLineHeight;
            
            std::vector<ci::gl::TextureRef> textures;
            std::vector<int> durations;
            int totalDuration = 0;
            for (auto textTime : textWithTime)
            {
                // string is first
                gl::TextureRef tex = textureForString(textTime.first, scale);
                textures.push_back(tex);
                
                // duration frames is second
                durations.push_back(textTime.second);
                totalDuration += textTime.second;
            }
            
            mTextures[contentID] = textures;
            mTextureDurations[contentID] = durations;
            mContentDurations[contentID] = totalDuration;
        }
    }

    gl::TextureRef TextLoopContent::textureForString(const std::string & str, const float scale)
    {
        // TODO: Account for alignment
        float x = 0;
        float y = 0;
        vector<string> tokens = ci::split(str, "\n");
        int lineCount = tokens.size();
        int maxChars = 0;
        for (string line : tokens)
        {
            if (line.size() > maxChars)
            {
                maxChars = line.size();
            }
        }
        
        Surface textSurf(maxChars * kCharPxWide,
                         lineCount * kCharPxTall,
                         true);
        // clear it out
        for (int x = 0; x < textSurf.getWidth(); ++x)
        {
            for (int y = 0; y < textSurf.getWidth(); ++y)
            {
                textSurf.setPixel(Vec2i(x,y), ColorAf(0,0,0,0));
            }
        }
        
        for(const char & c : str)
        {
            if (c == '\n')
            {
                y += kCharPxTall;
                x = 0;
                continue;
            }
            Rectf charRegion = regionForChar(c);
            textSurf.copyFrom(mFontSurf, ci::Area(charRegion), Vec2i((charRegion.x1 * -1) + x,
                                                                     (charRegion.y1 * -1) + y));
            x += kCharPxWide;
        }
        
        int newWidth = textSurf.getWidth() * scale;
        int newHeight = textSurf.getHeight() * scale;
        Surface scaledSurf(newWidth, newHeight, true);

        ip::resize(textSurf,
                   Area(0,0,textSurf.getWidth(),textSurf.getHeight()),
                   &scaledSurf,
                   Area(0,0,newWidth,newHeight));

        gl::Texture::Format texFormat;
        texFormat.magFilter( GL_LINEAR_MIPMAP_LINEAR ).minFilter( GL_LINEAR_MIPMAP_LINEAR ).mipMap().internalFormat( GL_RGBA );
        gl::Texture *frameTex = new gl::Texture(scaledSurf, texFormat);
        return gl::TextureRef(frameTex);
    }
    
    gl::TextureRef TextLoopContent::currentContentFrame()
    {
        // Find the approp frame
        std::vector<ci::gl::TextureRef> textures = mTextures[mContentID];
        std::vector<int> durations = mTextureDurations[mContentID];
        int totalDuration = mContentDurations[mContentID];
        int subFrame = mNumFramesRendered % totalDuration;
        int frameOffset = 0;
        gl::TextureRef tex;
        
        for (int i = 0; i < durations.size(); ++i)
        {
            int frameDuration = durations[i];
            frameOffset += frameDuration;
            if (subFrame <= frameOffset)
            {
                tex = textures[i];
                break;
            }
        }
        assert(tex);
        return tex;
    }
    
    void TextLoopContent::render(const ci::Vec2i & screenOffset, const ci::Rectf & contentRect)
    {
        gl::clear(ColorAf(0,0,0,0));
        gl::pushMatrices();
        gl::bindStockShader(gl::ShaderDef().color());
        Vec2i windowSize(contentRect.getWidth(), contentRect.getHeight());
        mCam.setOrtho(0, windowSize.x, windowSize.y, 0, 0, 100000);
        mCam.setAspectRatio((float)windowSize.x / (float)windowSize.y);
        gl::setMatrices(mCam);
        gl::enableAlphaBlending();
        gl::color(1,1,1,1);
        gl::setDefaultShaderVars();

        assert(mContentID > -1 && mContentID < 16000);
        
        gl::TextureRef tex = currentContentFrame();
        gl::draw(tex);

        gl::disableAlphaBlending();
        gl::popMatrices();
    }
	
}
