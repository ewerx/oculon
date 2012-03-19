//
//  Nbody.cl
//  Oculon
//
//  Created by Ehsan Rezaie on 12-03-06.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#define PARTICLE_FLAGS_INVSQR       0x02
#define PARTICLE_FLAGS_SHOW_DARK    0x04
#define PARTCILE_FLAGS_SHOW_SPEED   0x08

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
                      const uint flags)
                      //const float eps)
{
    // the 4th element of in_pos holds mass, this filters it out
    const float4 dt = (float4)(dt1,dt1,dt1,0.0f);
    const float eps = 0.0001f;
    //const float g = 1e-2;
    
	const uint i = get_global_id(0);
	if (i >= nb_particles)
		return;
    
	float4 p1 = in_pos[i];
	float4 a = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    
    // the higgs bassons
	const uint n = nb_particles / step;
    
	for(uint j = 0 ; j < n ; ++j)
	{
        const float4 p2 = in_pos[j];
        
		float4 d = p2 - p1;
        d.w = 0.0f;
        
        if( flags & PARTICLE_FLAGS_INVSQR )
        {
            float invr = rsqrt(d.x*d.x + d.y*d.y + d.z*d.z + eps);
            float f = /*g*p1.w**/p2.w*invr*invr*invr;
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
    
	a *= step; // what??
	float4 v = in_vel[i] + dt * a;
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
            color[i].w = 0.7f;
        }
        else
        {
            color[i].x = 1.0f;
            color[i].y = 1.0f;
            color[i].z = 1.0f;
            color[i].w = 0.7f;
        }
    }
    else if( flags & PARTCILE_FLAGS_SHOW_SPEED )
    {
        float speed = 10.0f*rsqrt(out_vel[i].x*out_vel[i].x + out_vel[i].y*out_vel[i].y + out_vel[i].z*out_vel[i].z);
        color[i].y = speed;
        color[i].z = speed;
        color[i].w = 0.7f;
    }
}
