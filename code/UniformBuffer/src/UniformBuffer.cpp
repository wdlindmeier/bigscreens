//
//  UniformBuffer.cpp
//  UniformBuffer
//
//  Created by Ryan Bartley on 11/11/13.
//
//

#include "UniformBuffer.h"

namespace bigscreens {
	
UboRef Ubo::create( GLenum target )
{
	return UboRef( new Ubo( target ) );
}

UboRef Ubo::create(GLenum target, GLsizeiptr allocationSize, GLenum usage)
{
	return UboRef( new Ubo( target, allocationSize, usage ) );
}

UboRef Ubo::create( GLenum target, GLsizeiptr allocationSize, const void *data, GLenum usage )
{
	return UboRef( new Ubo( target, allocationSize, data, usage ) );
}

Ubo::Ubo( GLenum target )
: BufferObj( target )
{
}

Ubo::Ubo( GLenum target, GLsizeiptr allocationSize, GLenum usage)
: BufferObj( target, allocationSize, usage)
{
    
}

Ubo::Ubo( GLenum target, GLsizeiptr allocationSize, const void *data, GLenum usage )
: BufferObj( target, allocationSize, data, usage )
{
}
	
}