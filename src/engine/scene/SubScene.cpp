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
using namespace oculon;

//
// SubScene
// 

SubScene::SubScene(Scene* scene, const std::string& name)
: NamedObject(name)
, mParentScene(scene)
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
    interface->gui()->addLabel(getName());
    interface->addParam(CreateBoolParam( "Active", &mIsActive )
                         .oscReceiver(getName(),"active"));
    
    
}