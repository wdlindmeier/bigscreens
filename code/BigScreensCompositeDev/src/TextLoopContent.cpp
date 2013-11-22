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
    ' ' // space is always last
};

const static int kCharCount = 49;
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
    
    void TextLoopContent::setTextForContentID(const std::string & str,
                                              const int contentID,
                                              const float absoluteLineHeight)
    {
        assert(mContentID > -1 && mContentID < 16000);

        if (mTextures.find(mContentID) == mTextures.end())
        {
            // We don't already have it. Create the texture.
            
            float scale = (float)absoluteLineHeight / (float)kHyperspaceNativeLineHeight;
            
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

            mTextures[contentID] = gl::TextureRef(frameTex);
        }
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

        gl::TextureRef tex = mTextures[mContentID];
        gl::draw(tex);

        gl::disableAlphaBlending();
        gl::popMatrices();
    }
	
}
