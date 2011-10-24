/*
 *  Behavior.h
 *  OculonProto
 *
 *  Created by Ehsan on 11-10-17.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __BEHAVIOR_H__
#define __BEHAVIOR_H__

class Entity;

class Behavior
{
public:
    Behavior();
    virtual ~Behavior();
    
    Entity* GetEntity() const { return mEntity; }
       
private:
    Entity* mEntity;
};

#endif // __BEHAVIOR_H__
