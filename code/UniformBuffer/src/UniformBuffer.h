//
//  UniformBuffer.h
//  UniformBuffer
//
//  Created by Ryan Bartley on 11/11/13.
//
//

#pragma once

#include "cinder/gl/BufferObj.h"
#include "cinder/gl/Context.h"

namespace bigscreens {
	
typedef std::shared_ptr<class Ubo> UboRef;
	
class Ubo : public ci::gl::BufferObj {
public:
	static UboRef	create( GLenum target = GL_UNIFORM_BUFFER );
    static UboRef   create( GLenum target, GLsizeiptr allocationSize, GLenum usage = GL_DYNAMIC_DRAW );
	static UboRef	create( GLenum target, GLsizeiptr allocationSize, const void *data, GLenum usage = GL_STATIC_DRAW );
	
	void setBlockIndex( const GLuint blockIndex )
	{
		mBlockIndex = blockIndex;
	}
	
	GLuint getBlockIndex() { return mBlockIndex; }

	void bindBufferBase()
	{
		glBindBufferBase( mTarget, mBlockIndex, mId );
	}
	
	void setOffsets( const std::string &name, const GLuint offset )
	{
		mOffsets.insert( std::pair<std::string, GLuint>( name, offset ) );
	}
	
	void bufferSubData( const std::string &name, const ci::ColorA &newColor )
	{
		auto data = mOffsets.find( name );
		if( data != mOffsets.end() ) {
			ci::gl::BufferScope bufferBind( mTarget, mId );
			ci::ColorA * color = (ci::ColorA*) glMapBufferRange( mTarget, data->second, sizeof(ci::ColorA), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT );
			*color = newColor;
			glUnmapBuffer( mTarget );
		}
		else {
			std::cout << "ERROR: " << name << " doesn't exist." << std::endl;
		}
	}
	
protected:
	Ubo( GLenum target = GL_UNIFORM_BUFFER );
	Ubo( GLenum target, GLsizeiptr allocationSize, GLenum usage = GL_DYNAMIC_DRAW );
	Ubo( GLenum target, GLsizeiptr allocationSize, const void *data, GLenum usage = GL_DYNAMIC_DRAW );
	
private:
	GLuint mBlockIndex;
	std::map<std::string, GLuint> mOffsets;
};
	
}