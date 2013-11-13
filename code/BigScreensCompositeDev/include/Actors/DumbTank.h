//
//  DumbTank.h
//  BigScreensComposite
//
//  Created by William Lindmeier on 11/10/13.
//
//


#include "cinder/gl/gl.h"
#include "cinder/ObjLoader.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"
#include "ObjModel.h"
#include "TankShot.h"
#include "Utilities.hpp"

#pragma once

namespace bigscreens
{
	
typedef std::shared_ptr<class DumbTank> DumbTankRef;

class DumbTank
{
    
public:
    
    DumbTank() { load(); }
    ~DumbTank(){ glDeleteTransformFeedbacks( 2, mTFOs ); }
    
    void load();
    void draw( const int zFactor, const ci::Vec3f & seperationPoint );
	void update( const ci::Vec3f & point );
    void loadShader();
	
private:
    
    
    void loadModels();
	
	ci::TriMeshRef		mMesh;
    ci::gl::VboRef		mFeedbackPositionVbo[2], mInitialTankPositionVbo, mTankPositionNormals;
    ci::gl::VboRef		mElementVbo;
    ci::gl::VaoRef		mVao[2];
	GLuint				mTFOs[2];

    ci::gl::GlslProgRef mRenderTankShader;
    
};
	
}