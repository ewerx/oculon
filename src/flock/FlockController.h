//
//  FlockController.h
//  Flocking
//
//  Created by Robert Hodgin on 4/26/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#pragma once
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "Lantern.h"
#include "Glow.h"
#include "Nebula.h"
#include <vector>

class Interface;

class FlockController {
public:
	FlockController();
	FlockController( int maxLanterns );
    void setupInterface( Interface* interface );
	void update(double dt);
	void updatePredatorBodies( ci::gl::Fbo *fbo );
	void drawLanterns( ci::gl::GlslProg *shader );
	void drawLanternGlows( const ci::Vec3f &right, const ci::Vec3f &up );
	void drawGlows( ci::gl::GlslProg *shader, const ci::Vec3f &right, const ci::Vec3f &up );
	void drawNebulas( ci::gl::GlslProg *shader, const ci::Vec3f &right, const ci::Vec3f &up );
	void addLantern( const ci::Vec3f &pos );
	void addGlows( Lantern *lantern, int amt );
	void addNebulas( Lantern *lantern, int amt );

	int						mNumLanterns;
	int						mMaxLanterns;
	std::vector<Lantern>	mLanterns;
	std::vector<Glow>		mGlows;
	std::vector<Nebula>		mNebulas;
    
    float mLanternPosX;
    
    bool mTick;
    float mTimer;
    
    static bool depthSortFunc( Lantern a, Lantern b );
};




