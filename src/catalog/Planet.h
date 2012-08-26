#pragma once
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Quaternion.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/Sphere.h"
#include <vector>

class Scene;
class Star;

class Planet {
public:
	Planet( Scene* scene,
           Star* hostStar,
           std::string name,
           float orbitalPeriod,
           float semiMajorAxis,
           float inclination,
           float mass,
           float radius
           );
	void update( const ci::Camera &cam, float scale );
	//void drawName( const ci::Vec2f &mousePos, float power, float alpha );

	std::string		mName;
	ci::gl::Texture mNameTex;
	
	float		mRadius;
    float       mOrbitalPeriod;
    float       mSemiMajorAxis;
    float       mInclination;
    float       mMass;
    
    Star*       mHostStar;
    Scene*      mParentScene;
};
