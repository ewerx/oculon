//
//  Nbody.cl
//  Oculon
//
//  Created by Ehsan Rezaie on 12-03-06.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#define PARTICLE_FLAGS_INVSQR       (1 << 0)
#define PARTICLE_FLAGS_SHOW_DARK    (1 << 1)
#define PARTICLE_FLAGS_SHOW_SPEED   (1 << 2)
#define PARTICLE_FLAGS_SHOW_MASS    (1 << 3)

#define MAX_MASS 50.0f

// pos.w = mass
// vel.w = unused

// based on Brown Deer N-body tutorial
__kernel void nbody_kern(
                         __global const float4* pos_old,
                         __global float4* pos_new,
                         __global float4* vel,
                         __local float4* pblock,
                         const float dt1, 
                         const float eps
                         ) 
{
    const float4 dt = (float4)(dt1,dt1,dt1,0.0f);
    
    int gti = get_global_id(0);
    int ti = get_local_id(0);
    
    int n = get_global_size(0);
    int nt = get_local_size(0);
    int nb = n/nt;
    
    float4 p = pos_old[gti];
    float4 v = vel[gti];
    
    float4 a = (float4)(0.0f,0.0f,0.0f,0.0f);
    
    for(int jb=0; jb < nb; jb++) 
    { /* Foreach block ... */
       pblock[ti] = pos_old[jb*nt+ti]; /* Cache ONE particle position */
       barrier(CLK_LOCAL_MEM_FENCE); /* Wait for others in the work-group */
    
        for(int j=0; j<nt; j++) 
        { /* For ALL cached particle positions ... */
            float4 p2 = pblock[j]; /* Read a cached particle position */
            float4 d = p2 - p;
            float invr = rsqrt(d.x*d.x + d.y*d.y + d.z*d.z + eps);
            float f = p2.w*invr*invr*invr;
            a += f*d; /* Accumulate acceleration */
        }
    
       barrier(CLK_LOCAL_MEM_FENCE); /* Wait for others in work-group */
    }
    
    p += dt*v + 0.5f*dt*dt*a;
    v += dt*a;
    
    pos_new[gti] = p;
    vel[gti] = v;
    
}

// based on FreeOCL sample
__kernel void gravity(__global const float4 *in_pos, 
                      __global float4 *out_pos,
					  __global const float4 *in_vel, 
                      __global float4 *out_vel,
                      __global float4 *color,
					  const uint nb_particles,
					  const uint step,
                      const float dt1,
                      const float damping,
                      const float gravity,
                      const float alpha,
                      const uint flags,
                      const float eps)
{
    // the 4th element of in_vel holds mass, this filters it out
    const float4 dt = (float4)(dt1,dt1,dt1,0.0f);
    //const float eps = eps;//150.f//0.0001f;
    
	const uint i = get_global_id(0);
	if (i >= nb_particles)
		return;
    
	float4 p1 = in_pos[i];
	float4 a = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    
    // the higgs bassons
	const uint n = step;
    
	for(uint j = 0 ; j < n ; ++j)
	{
		float4 d = in_pos[j] - p1;
        d.w = 0.0f;
        
        if( flags & PARTICLE_FLAGS_INVSQR )
        {
            float invr = rsqrt(d.x*d.x + d.y*d.y + d.z*d.z + eps);
            float f = /*in_vel[i].w */ gravity * in_vel[j].w * invr*invr;
            a += f*d;
        }
        else
        {
            d.w = 0.0f;
            float d2 = dot(d, d);
            d /= sqrt(d2);
            if (isnan(d.x) || isnan(d.y) || isnan(d.z) || i == j)
                continue;
            d.w = 0.0f;
            d2 = max(d2, 1e0f);
            a += (1.0f / d2) * d;
        }
	}
    
	float4 v = in_vel[i] + dt * a;
    float4 damping1 = (float4)(damping,damping,damping,1.0f);
    v *= damping1;
    out_pos[i] = p1 + (dt * v);
    
    out_vel[i] = v;
    
    if( flags & PARTICLE_FLAGS_INVSQR )
    {
        out_pos[i] += (0.5f*dt*dt*a); // what??
    }
    
    if( flags & PARTICLE_FLAGS_SHOW_DARK )
    {
        if( i < n )
        {
            color[i].x = 1.0f;
            color[i].y = 0.0f;
            color[i].z = 0.0f;
            color[i].w = alpha;
        }
        else
        {
            color[i].x = 1.0f;
            color[i].y = 1.0f;
            color[i].z = 1.0f;
            color[i].w = alpha;
        }
    }
    else if( flags & PARTICLE_FLAGS_SHOW_SPEED )
    {
        float speed = 10.0f*rsqrt(out_vel[i].x*out_vel[i].x + out_vel[i].y*out_vel[i].y + out_vel[i].z*out_vel[i].z);
        color[i].y = speed;
        color[i].z = speed;
        color[i].w = alpha;
    }
    else if( flags & PARTICLE_FLAGS_SHOW_MASS )
    {
        float massRatio = out_vel[i].w / MAX_MASS;
        color[i].x = massRatio*massRatio;
        color[i].y = massRatio/2.0f;
        color[i].z = massRatio;
        color[i].w = alpha;
        if( i < n )
        {
            color[i].w *= 2.0f;
        }
    }
}
