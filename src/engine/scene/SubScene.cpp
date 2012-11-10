/*
 *  SubScene.cpp
 *  Oculon
 *
 *  Created by Ehsan on 12-09-16.
 *  Copyright 2012 ewerx. All rights reserved.
 *
 */


#include "SubScene.h"
#include "Utils.h"
#include "Scene.h"
#include "OculonApp.h"
#include "Interface.h"

using namespace ci;
using namespace std;


//
// SubScene
// 

SubScene::SubScene(Scene* scene, const std::string& name)
: mParentScene(scene)
, mName(name)
, mIsActive(false)
{
}

SubScene::~SubScene()
{
}

void SubScene::setupInterface()
{
    Interface* interface = mParentScene->getInterface();
    interface->gui()->addColumn();
    interface->gui()->addLabel(mName);
    interface->addParam(CreateBoolParam( "Active", &mIsActive )
                         .oscReceiver(mName,"active"));
    
    
}