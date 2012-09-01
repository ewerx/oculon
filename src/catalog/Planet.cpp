/*
 *  Planet.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-19.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "Planet.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/Text.h"
#include "cinder/app/AppBasic.h"
#include "Scene.h"
#include "OculonApp.h"

using namespace ci;

Planet::Planet( Scene* scene,
               Star* hostStar,
               std::string name,
               float orbitalPeriod,
               float semiMajorAxis,
               float inclination,
               float eccentricity,
               float mass,
               float radius,
               float stellarMass )
: mParentScene(scene)
, mHostStar(hostStar)
, mName( name )
, mOrbitalPeriod(orbitalPeriod)
, mSemiMajorAxis(semiMajorAxis)
, mInclination(inclination)
, mEccentricity(eccentricity)
, mMass(mass)
, mRadius(radius)
, mStellarMass(stellarMass)
{
}

void Planet::update( const Camera &cam, float scale )
{
}
