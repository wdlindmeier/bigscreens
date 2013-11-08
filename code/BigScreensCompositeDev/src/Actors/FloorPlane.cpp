//
//  FloorPlane.cpp
//  ADJACENCY
//
//  Created by Ryan Bartley on 11/4/13.
//
//

#include "FloorPlane.h"

namespace bigscreens {

void FloorPlane::draw()
{
	static int index = 0;
	static float divideNum = 0.0f;
	
	mVao->bind();
	mLineElementVbo->bind();
	mNoiseTexture->bind();
	
	// This shader draws the colored quads
	mQuadTriangleGlsl->bind();
	
	mQuadTriangleGlsl->uniform( "projection", ci::gl::getProjection() );
	mQuadTriangleGlsl->uniform( "modelView", ci::gl::getModelView() );
	mQuadTriangleGlsl->uniform( "chooseColor", mChooseColor );
	mQuadTriangleGlsl->uniform( "colorOffset", index++ );
	mQuadTriangleGlsl->uniform( "heightMap", 0 );
	mQuadTriangleGlsl->uniform( "divideNum", divideNum );
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
	ci::gl::drawElements( GL_LINES_ADJACENCY, indexNum, GL_UNSIGNED_INT, 0 );
	
	mQuadTriangleGlsl->unbind();
	
	// This draws the wireframe
	mQuadOutlineGlsl->bind();
	
	mQuadOutlineGlsl->uniform( "projection", ci::gl::getProjection() );
	mQuadOutlineGlsl->uniform( "modelView", ci::gl::getModelView() );
	mQuadOutlineGlsl->uniform( "colorOffset", index++ );
	mQuadOutlineGlsl->uniform( "heightMap", 0 );
	//		mQuadTriangleGlsl->uniform( "divideNum", divideNum );
	
	ci::gl::drawElements( GL_LINES_ADJACENCY, indexNum, GL_UNSIGNED_INT, 0 );
	
	mQuadOutlineGlsl->unbind();
	
	mNoiseTexture->unbind();
	mLineElementVbo->unbind();
	mVao->unbind();
	
}
	
void FloorPlane::loadTexture()
{
	mNoiseTexture = ci::gl::Texture::create( ci::loadImage( ci::app::loadResource( "noise_map.png" ) ) );
}
	
void FloorPlane::loadShaders()
{
	// Terrain stuff
	ci::gl::GlslProg::Format mQuadOutlineFormat;
	mQuadOutlineFormat.vertex( ci::app::loadAsset( /*"quadOutline.vert"*/ SharedShaderAssetPath("quadOutline.vert", !IS_IAC) ) )
	.geometry( ci::app::loadAsset( /*"quadOutline.geom"*/ SharedShaderAssetPath("quadOutline.geom", !IS_IAC) ) )
	.fragment( ci::app::loadAsset( /*"quadOutline.frag"*/ SharedShaderAssetPath("quadOutline.frag", !IS_IAC) ) );
	mQuadOutlineGlsl = ci::gl::GlslProg::create( mQuadOutlineFormat );
	
	ci::gl::GlslProg::Format mQuadTriangleFormat;
	mQuadTriangleFormat.vertex( ci::app::loadAsset( /*"quadTriangle.vert"*/ SharedShaderAssetPath("quadTriangle.vert", !IS_IAC) ) )
	.geometry( ci::app::loadAsset( /*"quadTriangle.geom"*/ SharedShaderAssetPath("quadTriangle.geom", !IS_IAC) ) )
	.fragment( ci::app::loadAsset( /*"quadTriangle.frag"*/ SharedShaderAssetPath("quadOutline.frag", !IS_IAC) ) );
	mQuadTriangleGlsl = ci::gl::GlslProg::create( mQuadTriangleFormat );
	
}
	
void FloorPlane::createAndLoadGeometry()
{
	// this creates the points across x and y notice that we don't subtract 1 from each
	for( int y = 0; y < yCount; y++ ) {
		for( int x = 0; x < xCount; x++ ) {
			mTrimesh->appendVertex( ci::Vec3f( x*quadSize, y*quadSize, 0 ) );
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
	uint32_t * lineIndex = new uint32_t[indexNum];
	int index = 0;
	for( int i = 0; index < indexNum; i+=xCount ) {
		for( int j = 0; j < xCount-1; j++ ) {
			lineIndex[index+0] = i+j+xCount;
			lineIndex[index+1] = i+j+0;
			lineIndex[index+2] = i+j+1;
			lineIndex[index+3] = i+j+xCount+1;
			index+=4;
		}
	}
	
	mVao = ci::gl::Vao::create();
	mVao->bind();
	
	mVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, mTrimesh->getNumVertices() * sizeof( ci::Vec3f ), mTrimesh->getVertices<3>(), GL_STATIC_DRAW );
	mVbo->bind();
	
	ci::gl::enableVertexAttribArray(0);
	ci::gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
	
	mLineElementVbo = ci::gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, indexNum * sizeof(uint32_t), lineIndex, GL_STATIC_DRAW );
	
}

}