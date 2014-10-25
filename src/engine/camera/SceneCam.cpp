//
//  SceneCam.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-10-25.
//
//

#include "SceneCam.h"

using namespace ci;
using namespace std;

SceneCam::SceneCam(const std::string &name, const float aspectRatio)
: mName(name)
, mInterfacePanel(NULL)
{
}

SceneCam::~SceneCam()
{
}

void SceneCam::showInterfacePanel(bool show)
{
    if (mInterfacePanel)
    {
        mInterfacePanel->enabled = show;
    }
}
