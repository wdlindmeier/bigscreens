//
//  OutLineBorder.hpp
//  GridWindowRefactor
//
//  Created by Ryan Bartley on 10/14/13.
//
//

#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/GlslProg.h"
#include "Utilities.hpp"

namespace bigscreens {
	
typedef std::shared_ptr<class OutLineBorder> OutLineBorderRef;

class OutLineBorder {
public:
	OutLineBorder()
	{
		ci::gl::GlslProg::Format format;
		format.vertex( ci::app::loadResource( "outline.vert" ) )
			.fragment( ci::app::loadResource( "outline.frag" ) );
		mGlsl = ci::gl::GlslProg::create( format );
		mGlsl->bind();
		
		float vertices[] = {
			-1.0f,   1.0f,
			-1.0f,  -1.0f,
			-0.99f,	 0.99f,
			-0.99f, -0.99f,
			 1.0f,   1.0f,
			 1.0f,  -1.0f,
			 0.99f,  0.99f,
			 0.99f, -0.99f
		};
		
		GLuint elements[] = {
			0, 1, 2,
			2, 3, 1,
			0, 4, 2,
			2, 6, 4,
			4, 5, 6,
			6, 7, 5,
			5, 1, 7,
			7, 3, 1
		};
		
		mVao = ci::gl::Vao::create();
		mVao->bind();
		
		mVbo = ci::gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );
		mVbo->bind();
		
		auto position = mGlsl->getAttribLocation( "position" );
		ci::gl::vertexAttribPointer( position, 2, GL_FLOAT, GL_FALSE, 0, 0 );
		ci::gl::enableVertexAttribArray( position );
		
		mElementVbo = ci::gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements );
		mElementVbo->bind();
		
		mGlsl->unbind();
		mVao->unbind();
		mVbo->unbind();
		mElementVbo->unbind();
	}
	
	~OutLineBorder() {}
	
	void render()
	{
		mGlsl->bind();
			mVao->bind();
				mElementVbo->bind();
					ci::gl::drawElements( GL_LINES, 24, GL_UNSIGNED_INT, 0 );
				mElementVbo->unbind();
			mVao->unbind();
		mGlsl->unbind();
	}
	
private:
	ci::gl::VaoRef		mVao;
	ci::gl::VboRef		mVbo, mElementVbo;
	ci::gl::GlslProgRef mGlsl;
};
	
}
