//
//  ObjModel.h
//  AdvancedTank
//
//  Created by William Lindmeier on 11/3/13.
//
//

#include "cinder/gl/gl.h"
#include "cinder/ObjLoader.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"

#pragma once

namespace bigscreens
{
    class ObjModel
    {
        
    public:
        
        ObjModel(){};
        virtual ~ObjModel(){};
        
        void load(const std::string & objName, ci::gl::GlslProgRef & shader);
        virtual void render();
        
        ci::TriMeshRef &	getMesh();
        ci::gl::VboRef &	getVbo();
        ci::gl::VboRef &	getElementVbo();
        ci::gl::VaoRef &	getVao();
        
    protected:
        
        ci::TriMeshRef		mMesh;
        ci::gl::VboRef		mVbo;
        ci::gl::VboRef		mElementVbo;
        ci::gl::VaoRef		mVao;
        
    };
}