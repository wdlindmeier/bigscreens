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
#include "FiringTank.h"

#pragma once

namespace bigscreens
{

typedef std::shared_ptr<class DumbTank> DumbTankRef;

class DumbTank : public FiringTank
{
    
public:
    
    DumbTank(const std::string & objName, const float barrelAngleDegrees);
    ~DumbTank(){ glDeleteTransformFeedbacks( 2, mTFOs ); }
    
    void draw( const int zFactor, const ci::Vec3f & seperationPoint );
	void update(long progressCounter);
    void loadShader();
    
    ci::gl::VboRef getElementVbo();
    ci::gl::VaoRef getVao();
    ci::TriMeshRef getMesh();
	
private:
    
    
    void loadModels(const std::string & modelName);
	
	ci::TriMeshRef		mMesh;
    ci::gl::VboRef		mFeedbackPositionVbo[2], mInitialTankPositionVbo, mTankPositionNormals;
    ci::gl::VboRef		mElementVbo;
    ci::gl::VaoRef		mVao[2];
	GLuint				mTFOs[2];

    ci::gl::GlslProgRef mRenderTankShader;
    
};
	
}