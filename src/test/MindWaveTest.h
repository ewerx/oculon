/*
 *  MindWaveTest.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __MINDWAVETEST_H__
#define __MINDWAVETEST_H__

#include "cinder/Cinder.h"
#include "cinder/audio/Input.h"
#include "Scene.h"

//
// Audio input tests
//
class MindWaveTest : public Scene
{
public:
    MindWaveTest() {}
    virtual ~MindWaveTest() {}
    
    // inherited from Scene
    void setup();
    //void setupParams(params::InterfaceGl& params);
    //void reset();
    void update(double dt);
    void draw();
    
private:
    void drawGraphs();
    vector<float> mRawValues;
    
};

#endif // __MINDWAVETEST_H__