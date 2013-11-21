//
//  FloorPlane.cpp
//  ADJACENCY
//
//  Created by Ryan Bartley on 11/4/13.
//
//

#include "FloorPlane.h"
#include "ContentProvider.h"

using namespace ci;

namespace bigscreens
{

FloorPlane::FloorPlane(const ci::Vec2i & size) : //, const float scale) :
mSize(size)
, mIndexCount((size.x-1)*(size.y-1) * 4)
, mNearLimit( 150 )
, mFarLimit( 300 )
{
    ci::TriMesh::Format mTriFormat;
    mTriFormat.positions(3);
    mTrimesh = ci::TriMesh::create( mTriFormat );
    
    loadTexture();
    createAndLoadGeometry();
    loadShaders();
}

void FloorPlane::draw(const long framesRendered,
                      const float alpha,
                      const float mountainMultiplier,
                      const ci::Vec3f & groundScale,
                      const ci::Vec3f & groundOffset,
                      const ci::Vec3f & tankPosition,
                      const ci::Vec3f & tankVector)
{
    mVao->bind();
    mLineElementVbo->bind();
    mNoiseTexture->bind();
    
    gl::enableAlphaBlending();
    
    float *fft = SceneContentProvider::sharedContentProvider()->getFFTData();

    //if (shouldRenderColor)
    {
        
        // This shader draws the colored quads
        mQuadTriangleGlsl->bind();
        
        TryAddingUniform(mQuadTriangleGlsl, "projection", ci::gl::getProjection());
        TryAddingUniform(mQuadTriangleGlsl, "modelView", ci::gl::getModelView() );
        
        TryAddingUniform(mQuadTriangleGlsl, "alphaMultiplier", alpha);
        TryAddingUniform(mQuadTriangleGlsl, "groundScale", groundScale);
        TryAddingUniform(mQuadTriangleGlsl, "groundOffset", groundOffset);
        TryAddingUniform(mQuadTriangleGlsl, "mountainMultiplier", mountainMultiplier);
        
        TryAddingUniform(mQuadTriangleGlsl, "chooseColor", true);
        TryAddingUniform(mQuadTriangleGlsl, "colorOffset", (int)framesRendered );
        TryAddingUniform(mQuadTriangleGlsl, "heightMap", 0);
        TryAddingUniform(mQuadTriangleGlsl, "dimensions", mSize);
        TryAddingUniform(mQuadTriangleGlsl, "farLimit", mFarLimit );
        TryAddingUniform(mQuadTriangleGlsl, "nearLimit", mNearLimit );
        TryAddingUniform(mQuadTriangleGlsl, "tankPosition", tankPosition );
        TryAddingUniform(mQuadTriangleGlsl, "tankVector", tankVector );
        try { mQuadTriangleGlsl->uniform("fft", fft, kNumFFTChannels); } catch (cinder::gl::GlslUnknownUniform){}
        
        //	mQuadTriangleGlsl->uniform( "divideNum", divideNum );
        // lines adjacency gives the geometry shader two points on either side of the line
        // this was the problem with the indexing, you have to give two other points when
        // using it. basically from above
        //
        // index0 - - - - index1 ------------ index2 - - - - index3
        //
        // if I just drew GL_LINES it wouldn't use index0 or index3 in one geometry shader
        // instance. basically you get the four points of a quad and can make the triangle_strip
        // or line_strip like i do with the shaders
        // instead it would be...
        //
        // index0 --------- index1 // one geometry shader instance
        // index1 --------- index2 // another geometry shader instance
        // etc.
        ci::gl::drawElements( GL_LINES_ADJACENCY, mIndexCount, GL_UNSIGNED_INT, 0 );
        
        mQuadTriangleGlsl->unbind();
    }

	// This draws the wireframe
	mQuadOutlineGlsl->bind();

    TryAddingUniform(mQuadOutlineGlsl, "dimensions", mSize);
	TryAddingUniform(mQuadOutlineGlsl, "projection", ci::gl::getProjection() );
	TryAddingUniform(mQuadOutlineGlsl, "modelView", ci::gl::getModelView() );
    
    TryAddingUniform(mQuadOutlineGlsl, "alphaMultiplier", alpha);
    TryAddingUniform(mQuadOutlineGlsl, "groundScale", groundScale);
    TryAddingUniform(mQuadOutlineGlsl, "groundOffset", groundOffset);
    TryAddingUniform(mQuadOutlineGlsl, "mountainMultiplier", mountainMultiplier);

    //TryAddingUniform(mQuadOutlineGlsl, "uColor", colorOutline );
    TryAddingUniform(mQuadOutlineGlsl, "farLimit", mFarLimit );
    TryAddingUniform(mQuadOutlineGlsl, "nearLimit", mNearLimit );
    TryAddingUniform(mQuadOutlineGlsl, "tankPosition", tankPosition );
    TryAddingUniform(mQuadOutlineGlsl, "tankVector", tankVector );
	TryAddingUniform(mQuadOutlineGlsl, "heightMap", 0 );
    try { mQuadOutlineGlsl->uniform("fft", fft, kNumFFTChannels); } catch (cinder::gl::GlslUnknownUniform){}

	ci::gl::drawElements( GL_LINES_ADJACENCY, mIndexCount, GL_UNSIGNED_INT, 0 );

    mQuadOutlineGlsl->unbind();
    
	mNoiseTexture->unbind();
	mLineElementVbo->unbind();
	mVao->unbind();
	
}
	
void FloorPlane::loadTexture()
{
	mNoiseTexture = ci::gl::Texture::create( ci::loadImage( ci::app::loadResource( "noise_map.png" ) ) );
}
    
void FloorPlane::setNoiseTexture(ci::gl::TextureRef & tex)
{
    mNoiseTexture = tex;
}
    
void FloorPlane::setFarLimit( float farLimit )
{
    mFarLimit = farLimit;
}
    
void FloorPlane::setNearLimit( float nearLimit )
{
    mNearLimit = nearLimit;
}
	
void FloorPlane::loadShaders()
{
	// Terrain stuff
	ci::gl::GlslProg::Format mQuadOutlineFormat;
	mQuadOutlineFormat.vertex( ci::app::loadResource("quadOutline.vert") )
	.geometry( ci::app::loadResource("quadOutline.geom") )
	.fragment( ci::app::loadResource("quadOutline.frag") );
	mQuadOutlineGlsl = ci::gl::GlslProg::create( mQuadOutlineFormat );
	
	ci::gl::GlslProg::Format mQuadTriangleFormat;
	mQuadTriangleFormat.vertex( ci::app::loadResource("quadTriangle.vert") )
	.geometry( ci::app::loadResource("quadTriangle.geom") )
	.fragment( ci::app::loadResource("quadTriangle.frag") );
	mQuadTriangleGlsl = ci::gl::GlslProg::create( mQuadTriangleFormat );
	
}
	
void FloorPlane::createAndLoadGeometry()
{
	// this creates the points across x and y notice that we don't subtract 1 from each
    
    // NOTE: -1 so the planes abut when they're tiled, since we're not drawing the last one
    float quadScaleX = 1.0f / (mSize.x-1);
    float quadScaleY = 1.0f / (mSize.y-1);
    
	for( int z = 0; z < mSize.y; z++ )
    {
		for( int x = 0; x < mSize.x; x++ )
        {
			mTrimesh->appendVertex( ci::Vec3f(x * quadScaleX,
                                              0,
                                              z * quadScaleY ) );
		}
	}
	// this creates the index and this was the hardest part,
	// it starts at one quadsize above which is the total xCount,
	// then goes one quadsize below which would be the current x coordinate
	// then goes one quadsize to the right, and then one quadsize up from that
	//            index0          index3
	//                  |         |
	//  quadsize        |         |
	//                  |         |
	//                  |_________|
	//            index1          index2
	//                   quadsize
	// we use up to xcount - 1 because we skip the last point to create the last quad
	//
	uint32_t * lineIndex = new uint32_t[mIndexCount];
	int index = 0;
	for( int i = 0; index < mIndexCount; i+=mSize.x ){
		for( int j = 0; j < mSize.x-1; j++ ){
			lineIndex[index+0] = i+j+mSize.x;
			lineIndex[index+1] = i+j+0;
			lineIndex[index+2] = i+j+1;
			lineIndex[index+3] = i+j+mSize.x+1;
			index+=4;
		}
	}
	
	mVao = ci::gl::Vao::create();
	mVao->bind();
	
	mVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mTrimesh->getNumVertices() * sizeof( ci::Vec3f ), mTrimesh->getVertices<3>(), GL_STATIC_DRAW );
	mVbo->bind();
	
	ci::gl::enableVertexAttribArray(0);
	ci::gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	
	mLineElementVbo = ci::gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, mIndexCount * sizeof(uint32_t), lineIndex, GL_STATIC_DRAW );
}

}