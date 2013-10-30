#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Vbo.h"
#include "cinder/TriMesh.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;
          // how many across, how many up,  how many units between, element number to create the quads
          //                                                        this was the magic sauce for the elements
const int    xCount = 100,    yCount = 100, quadSize = 10,          indexNum = (xCount-1)*(yCount-1) * 4;

class ADJACENCYApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    void loadShaders();
    
    gl::VaoRef mVao;
    gl::VboRef mVbo, mLineElementVbo;
    gl::GlslProgRef mQuadOutlineGlsl, mQuadTriangleGlsl;
    TriMeshRef mTrimesh;
    CameraPersp mCam;
};

void ADJACENCYApp::setup()
{
    TriMesh::Format mFormat;
    mFormat.vertices(3);
    mTrimesh = TriMesh::create( mFormat );
    
    // this creates the points across x and y notice that we don't subtract 1 from each
    for( int y = 0; y < yCount; y++ ) {
        for( int x = 0; x < xCount; x++ ) {
            mTrimesh->appendVertex( Vec3f( x*quadSize, y*quadSize, 0 ) );
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
    
    mVao = gl::Vao::create();
    mVao->bind();
    
    mVbo = gl::Vbo::create( GL_ARRAY_BUFFER, mTrimesh->getNumVertices() * sizeof( Vec3f ), mTrimesh->getVertices<3>(), GL_STATIC_DRAW );
    mVbo->bind();
    
    gl::enableVertexAttribArray(0);
    gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    
    mLineElementVbo = gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, indexNum * sizeof(uint32_t), lineIndex, GL_STATIC_DRAW );
    
    mCam.setPerspective( 60, getWindowWidth() / getWindowHeight(), .1, 1000 );
    mCam.lookAt( Vec3f( 0, 0, 1 ), Vec3f( 0, 0, 0 ) );
    
    loadShaders();
}

void ADJACENCYApp::loadShaders()
{
    gl::GlslProg::Format mQuadOutlineFormat;
    mQuadOutlineFormat.vertex( loadAsset( "quadOutline.vert" ) )
    .geometry( loadAsset( "quadOutline.geom") )
    .fragment( loadAsset( "quadOutline.frag" ) );
    mQuadOutlineGlsl = gl::GlslProg::create( mQuadOutlineFormat );
    
    gl::GlslProg::Format mQuadTriangleFormat;
    mQuadTriangleFormat.vertex( loadAsset( "quadTriangle.vert" ) )
    .geometry( loadAsset( "quadTriangle.geom") )
    .fragment( loadAsset( "quadTriangle.frag" ) );
    mQuadTriangleGlsl = gl::GlslProg::create( mQuadTriangleFormat );
}

void ADJACENCYApp::mouseDown( MouseEvent event )
{
    loadShaders();
}

void ADJACENCYApp::update()
{
}

void ADJACENCYApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::pushMatrices();
    gl::setMatrices( mCam );
    // simple centering
    gl::multModelView( Matrix44f::createTranslation( Vec3f( -((xCount-1)*quadSize) / 2, -((yCount-1)*quadSize) / 2, -(xCount-1) * quadSize ) ) );

    mVao->bind();
    mLineElementVbo->bind();
    
    // This shader draws the colored quads
        mQuadTriangleGlsl->bind();
    
        mQuadTriangleGlsl->uniform( "projection", gl::getProjection() );
        mQuadTriangleGlsl->uniform( "modelView", gl::getModelView() );
    
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
            gl::drawElements( GL_LINES_ADJACENCY, indexNum, GL_UNSIGNED_INT, 0 );
    
        mQuadTriangleGlsl->unbind();
    
    // This draws the wireframe
        mQuadOutlineGlsl->bind();
    
        mQuadOutlineGlsl->uniform( "projection", gl::getProjection() );
        mQuadOutlineGlsl->uniform( "modelView", gl::getModelView() );
    
            gl::drawElements( GL_LINES_ADJACENCY, indexNum, GL_UNSIGNED_INT, 0 );
    
        mQuadOutlineGlsl->unbind();
    
    mLineElementVbo->unbind();
    mVao->unbind();
    
    
    gl::popMatrices();
    
}

CINDER_APP_NATIVE( ADJACENCYApp, RendererGl )
