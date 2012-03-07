//
//  Nbody.cl
//  Oculon
//
//  Created by Ehsan Rezaie on 12-03-06.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

// pos.w = mass
// vel.w = unused

__kernel void nbody_kern(
                         float dt1, float eps,
                         __global float4* pos_old,
                         __global float4* pos_new,
                         __global float4* vel,
                         __local float4* pblock
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

#define dt 1e-2f
__kernel void gravity(__global const float4 *in_pos, 
                      __global float4 *out_pos,
					  __global const float4 *in_vel, 
                      __global float4 *out_vel,
					  const uint nb_particles,
					  const uint step)
{
	const uint i = get_global_id(0);
	if (i >= nb_particles)
		return;
	float4 pos = in_pos[i];
	float4 a = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
	const uint n = nb_particles / step;
	for(uint j = 0 ; j < n ; ++j)
	{
		float4 p = in_pos[j] - pos;
		p.w = 0.0f;
		float d2 = dot(p, p);
		p /= sqrt(d2);
		if (isnan(p.x) || isnan(p.y) || isnan(p.z) || i == j)
			continue;
		p.w = 0.0f;
		d2 = max(d2, 1e0f);
		a += (1.0f / d2) * p;
	}
	a *= step;
	float4 v = in_vel[i] + dt * a;
	out_vel[i] = v;
	out_pos[i] = pos + dt * v;
}
