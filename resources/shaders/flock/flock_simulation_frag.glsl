#extension GL_EXT_gpu_shader4: enable

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D information;
uniform sampler2D oVelocities;
uniform sampler2D oPositions;

uniform float particleBufSize;

uniform float dt;
uniform bool reset;
uniform bool startAnim;
uniform float formationStep;

uniform sampler2D predatorPositionTex;
uniform float predatorBufSize;

uniform sampler2D audioData;
uniform float gain;

varying vec4 texCoord;

void reactToLanterns( inout vec3 acc, vec3 _myPos )
{
    float index = invNumLightsHalf;
    for( float i=0.0; i<numLights; i+=1.0 ){
        vec4 LANTERN	= texture2D( lanternsTex, vec2( index, 0.25 ) );
        vec3 pos		= LANTERN.xyz;
        float radius	= LANTERN.w;
        float minRad	= ( radius + 50.0 ) * ( radius + 50.0 );
        float maxRad	= ( radius + 350.0 ) * ( radius + 350.0 );
        
        vec3 dirToLantern		= _myPos - pos;
        float distToLantern		= length( dirToLantern );
        float distToLanternSqrd	= distToLantern * distToLantern;
        
        // IF WITHIN THE ZONE, REACT TO THE LANTERN
        if( distToLanternSqrd > minRad && distToLanternSqrd < maxRad ){
            acc -= normalize( dirToLantern ) * ( ( maxRad - minRad ) / distToLanternSqrd ) * 0.01075 * dt;
        }
        
        // IF TOO CLOSE, MOVE AWAY MORE RAPIDLY
        if( distToLantern < radius * 1.1 )
            acc += normalize( dirToLantern );
        
        index			+= invNumLights;
    }
}



void reactToPredators( inout vec3 acc, inout float crowded, vec3 fishPos )
{
    for( int y=0; y<pFboDim; y++ ){
        for( int x=0; x<pFboDim; x++ ){
            vec2 tc					= vec2( float(x), float(y) ) * pInvFboDim + pInvFboDim * 0.5;
            vec3 predatorPos		= texture2D( predatorPositionTex, tc ).rgb;
            float predatorZoneRadius	= 90.0 * 90.0;
            
            vec3 dirToPredator			= fishPos - predatorPos;
            float distToPredator		= length( dirToPredator );
            float distToPredatorSqrd	= distToPredator * distToPredator;
            
            if( distToPredatorSqrd < predatorZoneRadius ){
                float per = predatorZoneRadius / ( distToPredatorSqrd + 1.0 );
                crowded += per;
                acc += normalize( dirToPredator ) * per * 0.12 * dt;
            }
        }
    }
}


void main()
{
    // REALTIME MAC LAPTOP
    float zoneRadius	 = 30.0;
    float zoneRadiusSqrd = zoneRadius * zoneRadius;
    float minThresh		 = 0.44;
    float maxThresh		 = 0.90;
    float maxSpeed		 = 4.1;
    float crowdMulti	 = 0.4;
    
    // LARGE POPULATION RENDERS
    //	float zoneRadius	 = 15.0;
    //	float zoneRadiusSqrd = zoneRadius * zoneRadius;
    //	float minThresh		 = 0.44;
    //	float maxThresh		 = 0.90;
    //	float maxSpeed		 = 4.1;
    //	float crowdMulti	 = 0.4;
    
    vec4 vPos = texture2D( positions, texCoord.st )
	vec3 pos = vPos.rgb;
	float leadership = vPos.a;

    vec4 vVel = texture2D( velocities, texCoord.st )
	vec3 vel = vVel.rgb;
	float crowd = vVel.a;
    
    vec3 acc			= vec3( 0.0, 0.0, 0.0 );
    float offset		= invFboDim * 0.5;
    
    int myX				= int( texCoord.s * float(particleBufSize) );
    int myY				= int( texCoord.t * float(particleBufSize) );
    float crowded		= 2.0;
    
    vec3 startPos = texture2D( information, texCoord.st ).rgb;
	float nodeIndex = texture2D( information, texCoord.st ).a;
    
    bool doSim = true; // to skip simulation when forcing containment...
    float dist = length(pos);
    
    // animate to formation
    if (startAnim)
    {
        startPos = pos;
    }
    
    if (formationStep < 1.0)
    {
        //vel = vec3(0.0,0.0,0.0);
        vec3 targetPos = texture2D( oPositions, texCoord.st ).rgb;
        
        pos = mix(startPos,targetPos,formationStep);
    }
    else if (doSim) // simulate motion
    {
        // APPLY THE ATTRACTIVE, ALIGNING, AND REPULSIVE FORCES
        for( int y=0; y<fboDim; y++ ){
            for( int x=0; x<fboDim; x++ ){
                if( x == myX && y == myY ){
                    // Avoid comparing my sphere against my sphere
                } else {
                    vec2 tc			= vec2( float(x), float(y) ) * invFboDim + offset;
                    vec4 pos		= texture2D( positionTex, tc );
                    vec3 dir		= myPos - pos.xyz;
                    
                    float dist		= length( dir );
                    float distSqrd	= dist * dist;
                    
                    vec3 dirNorm	= normalize( dir );
                    
                    if( distSqrd < zoneRadiusSqrd ){
                        float percent = distSqrd/zoneRadiusSqrd + 0.0000001;
                        crowded += ( 1.0 - percent ) * crowdMulti;
                        
                        // IF FISH IS CLOSE, REPEL
                        if( percent < minThresh ){
                            float F  = ( minThresh/percent - 1.0 );
                            acc		+= dirNorm * F * 0.1 * dt * leadership;
                            
                            // IF FISH IS IN THE SWEET SPOT, ALIGN
                        } else if( percent < maxThresh ){
                            float threshDelta		= maxThresh - minThresh;
                            float adjustedPercent	= ( percent - minThresh )/( threshDelta + 0.0000001 );
                            float F					= ( 1.0 - ( cos( adjustedPercent * 6.28318 ) * -0.5 + 0.5 ) );
                            
                            acc += normalize( texture2D( velocityTex, tc ).xyz ) * F * 0.1 * dt * leadership;
                            
                            // IF FISH IS FAR, BUT WITHIN THE ACCEPTABLE ZONE, ATTRACT
                        } else if( dist < zoneRadius ){
                            
                            float threshDelta		= 1.0 - maxThresh;
                            float adjustedPercent	= ( percent - maxThresh )/( threshDelta + 0.0000001 );
                            float F					= ( 1.0 - ( cos( adjustedPercent * 6.28318 ) * -0.5 + 0.5 ) ) * 0.1 * dt * leadership;
                            
                            acc -= dirNorm * F;
                            
                        }
                    }
                }
            }
        }
        
        reactToLanterns( acc, myPos );
        
        reactToPredators( acc, crowded, myPos );
        
        
        myCrowd -= ( myCrowd - crowded ) * ( 0.1 * dt );
        
        // MODULATE MYCROWD MULTIPLIER AND GRAVITY FOR INTERESTING DERIVATIONS
        
        myVel += acc * dt;
        float newMaxSpeed = maxSpeed + myCrowd * 0.03;			// CROWDING MAKES EM FASTER
        
        float velLength = length( myVel );						// GET READY TO IMPOSE SPEED LIMIT
        if( velLength > newMaxSpeed ){							// SPEED LIMIT FOR FAST
            myVel = normalize( myVel ) * newMaxSpeed;
        }
        
        
        // MAIN GRAVITY TO MAKE THEM FALL
        //	myVel += vec3( 0.0, -0.0025, 0.0 );
        
        // TODO: containment/walls ?
        /*
        vec3 tempNewPos		= myPos + myVel * dt;		// NEXT POSITION
        
        
        // AVOID WALLS
        //if( power > 0.5 ){
        float xPull	= tempNewPos.x/( roomBounds.x );
        float yPull	= tempNewPos.y/( roomBounds.y );
        float zPull	= tempNewPos.z/( roomBounds.z );
        myVel -= vec3( xPull * xPull * xPull * xPull * xPull,
                      yPull * yPull * yPull * yPull * yPull,
                      zPull * zPull * zPull * zPull * zPull ) * 0.1;
        //}
        
        bool hitWall = false;
        vec3 wallNormal = vec3( 0.0 );
        float myRadius = 4.0;
        
        if( tempNewPos.y - myRadius < -roomBounds.y ){
            hitWall = true;
            wallNormal += vec3( 0.0, 1.0, 0.0 );
        } else if( tempNewPos.y + myRadius > roomBounds.y ){
            hitWall = true;
            wallNormal += vec3( 0.0,-1.0, 0.0 );
        }
        
        if( tempNewPos.x - myRadius < -roomBounds.x ){
            hitWall = true;
            wallNormal += vec3( 1.0, 0.0, 0.0 );
        } else if( tempNewPos.x + myRadius > roomBounds.x ){
            hitWall = true;
            wallNormal += vec3(-1.0, 0.0, 0.0 );
        }
        
        if( tempNewPos.z - myRadius < -roomBounds.z ){
            hitWall = true;
            wallNormal += vec3( 0.0, 0.0, 1.0 );
        } else if( tempNewPos.z + myRadius > roomBounds.z ){
            hitWall = true;
            wallNormal += vec3( 0.0, 0.0,-1.0 );
        }
        
        // WARNING, THIS MAY BE FAULTY MATH. MIGHT EXPLAIN LOST PARTICLES
        if( hitWall ){
            vec3 reflect = 2.0 * wallNormal * ( wallNormal * myVel );
            myVel -= reflect * 0.65;
        }
         */
    }
	
    // reincarnation
	if( reset )
    {
        pos = texture2D(oPositions, texCoord.st).rgb;
        vel = texture2D(oVelocities, texCoord.st).rgb;
    }
	
    //position + mass
	gl_FragData[0] = vec4(pos, leadership);
    //velocity + nodeIndex
	gl_FragData[1] = vec4(vel, crowd);
    //age information
	gl_FragData[2] = vec4(startPos, nodeIndex);
}


