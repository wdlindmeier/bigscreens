//
//  ObjModel.cpp
//  AdvancedTank
//
//  Created by William Lindmeier on 11/3/13.
//
//

#include "ObjModel.h"

using namespace ci;
using namespace ci::app;
using namespace bigscreens;

void ObjModel::load(const std::string & objName, ci::gl::GlslProgRef & shader)
{
    DataSourceRef file = loadResource( objName );
    ObjLoader loader( file );
    mMesh = TriMesh::create( loader );
    
    mVao = gl::Vao::create();
    mVao->bind();
    
    mVbo = gl::Vbo::create(GL_ARRAY_BUFFER, 3 * mMesh->getNumVertices() * sizeof(float),
                               mMesh->getVertices<3>(), GL_STATIC_DRAW );
    mVbo->bind();
    
    GLint pos = shader->getAttribSemanticLocation( geom::Attrib::POSITION ); //"vPosition" );
    gl::enableVertexAttribArray( pos );
    gl::vertexAttribPointer( pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    mElementVbo = gl::Vbo::create(GL_ELEMENT_ARRAY_BUFFER,
                                  mMesh->getNumIndices() * 4,
                                  mMesh->getIndices().data());
    mElementVbo->bind();
    mElementVbo->unbind();
    
    mVbo->unbind();
    mVao->unbind();
}

void ObjModel::render()
{
    // start
    mVao->bind();
    mElementVbo->bind();
    
    gl::setDefaultShaderVars();
    
    int numIndices = mMesh->getNumIndices();
    
    gl::drawElements(GL_LINES,
                     numIndices,
                     GL_UNSIGNED_INT,
                     0);
    
    mElementVbo->unbind();
    mVao->unbind();
}