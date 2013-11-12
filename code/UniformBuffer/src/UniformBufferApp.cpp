#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Context.h"

#include "UniformBuffer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class UniformBufferApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	
	gl::GlslProgRef mGlsl, mGlsl2;
	bigscreens::UboRef mUniformBuffer;
	gl::VaoRef			mVao;
	gl::VboRef			mVbo, mElementVbo;
};

void UniformBufferApp::setup()
{
	mVao = gl::Vao::create();
	mVao->bind();
	
	const float verts[] = {
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f
	};
	
	const GLuint index[] = {
		0, 1, 2,
		0, 2, 3,
	};
	
	mVbo = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW );
	mVbo->bind();
	gl::enableVertexAttribArray(0);
	gl::vertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0 );
	gl::enableVertexAttribArray( 1 );
	gl::vertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)) );
	
	mElementVbo = gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), index, GL_STATIC_DRAW );
	
	
	mGlsl = gl::GlslProg::create( gl::GlslProg::Format()
								 .vertex( loadAsset( "uniformTest.vert") )
								 .fragment( loadAsset( "uniformTest.frag") ) );
	mGlsl2 = gl::GlslProg::create( gl::GlslProg::Format()
								 .vertex( loadAsset( "uniformTest2.vert") )
								 .fragment( loadAsset( "uniformTest2.frag") ) );
	mUniformBuffer = bigscreens::Ubo::create();
	mUniformBuffer->setBlockIndex( mGlsl->getUniformBlockIndex( "BlobSettings" ) );
	
	cout << "This is mGlsl1 index: " << mGlsl->getUniformBlockIndex( "BlobSettings" ) << "This is mGlsl2 index: " << mGlsl2->getUniformBlockIndex( "BlobSettings" ) << endl;
	
	cout << mUniformBuffer->getBlockIndex() << endl;
	
	GLint blockSize;
	mGlsl->queryActiveUniformBlock( mUniformBuffer->getBlockIndex(), GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize );
	
	GLubyte * blockBuffer = (GLubyte *) malloc( blockSize );
	
	cout << blockSize << endl;
	
	const GLchar * names[] = { "InnerColor", "OuterColor", "RadiusInner", "RadiusOuter" };
	
	GLuint indices[4];
	glGetUniformIndices( mGlsl->getHandle(), 4, names, indices );
	
	GLint offset[4];
	glGetActiveUniformsiv( mGlsl->getHandle(), 4, indices, GL_UNIFORM_OFFSET, offset );
	
	ColorA innerColor( 1.0f, 1.0f, 0.75f, 1.0f );
	ColorA outerColor( 0.0f, 0.0f, 0.0f, 0.0f );
	float innerRadius = 0.25f; float outerRadius = 0.45f;
	
	memcpy(blockBuffer + offset[0], innerColor, sizeof(ColorA));
	memcpy(blockBuffer + offset[1], outerColor, sizeof(ColorA));
	memcpy(blockBuffer + offset[2], &innerRadius, sizeof(float));
	memcpy(blockBuffer + offset[3], &outerRadius, sizeof(float));
	
	mUniformBuffer->bufferData( blockSize, blockBuffer, GL_DYNAMIC_DRAW );
	mUniformBuffer->bindBufferBase();
	
	mUniformBuffer->setOffsets("InnerColor", offset[0]);
	mUniformBuffer->setOffsets("OuterColor", offset[1]);
}

void UniformBufferApp::mouseDown( MouseEvent event )
{
}

void UniformBufferApp::update()
{
	static float r = 0.0f, g = 0.0f, b = 0.0f;
	
	r+=.01f; g-=.01; b+=.001;
	
	if(r > 1.0f)
		r = 0.0f;
	if(g < 0.0f)
		g = 1.0f;
	if(b > 1.0f)
		b = 0.0;
	
	mUniformBuffer->bufferSubData( "OuterColor", ColorA( r, g, b, 1.0f ) );
	mUniformBuffer->bufferSubData( "InnerColor", ColorA( g, r, b, 1.0f ) );
}

void UniformBufferApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	gl::viewport( Vec2i( 0, 0 ), Vec2i( getWindowWidth() / 2, getWindowHeight() ) );
	mGlsl2->bind();
	mVao->bind();
	mElementVbo->bind();
	
	gl::drawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	mGlsl2->unbind();
	
	gl::viewport( Vec2i( getWindowWidth() / 2, 0 ), Vec2i( getWindowWidth() / 2, getWindowHeight() ) );
	mGlsl->bind();

	
	gl::drawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	mElementVbo->unbind();
	mVao->unbind();
	mGlsl->unbind();
}

CINDER_APP_NATIVE( UniformBufferApp, RendererGl )
