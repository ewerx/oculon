#define DAMP			0.95f
#define CENTER_FORCE	0.007f
#define MOUSE_FORCE		300.0f
#define MIN_SPEED		0.1f
#define MAX_MASS        1.0f

#define PARTICLE_FLAGS_INVSQR       (1 << 0)
#define PARTICLE_FLAGS_SHOW_DARK    (1 << 1)
#define PARTICLE_FLAGS_SHOW_SPEED   (1 << 2)
#define PARTICLE_FLAGS_SHOW_MASS    (1 << 3)

typedef struct
{
	float2 vel;
	float mass;
	float dummy;		// need this to make sure the float2 vel is aligned to a 16 byte boundary
} clParticle;

typedef struct
{
    float2 mVel;
    float mMass;
    float mLife;    
    // make sure the float2 vel is aligned to a 16 byte boundary
} tParticle;

typedef struct 
{
    float2 mPos;
    float2 mVel;
    float mMass;
    float mCharge;
} tNode;

// MSALibs Demo
__kernel void updateParticle(__global clParticle* particles, __global float2* posBuffer, const float2 mousePos, const float2 dimensions)
{
	int id = get_global_id(0);
	__global clParticle *p = &particles[id];
	
	float2 diff = mousePos - posBuffer[id];
	float invDistSQ = 1.0f / dot(diff, diff);
	diff *= MOUSE_FORCE * invDistSQ;

	p->vel += (dimensions*0.5f - posBuffer[id]) * CENTER_FORCE - diff* p->mass;
	
	float speed2 = dot(p->vel, p->vel);
	if(speed2<MIN_SPEED) posBuffer[id] = mousePos + diff * (1 + p->mass);

	posBuffer[id] += p->vel;
	p->vel *= DAMP;
}

// Magneto
__kernel void magnetoParticle(__global tParticle* particles, 
                              __global tNode* nodes,
                              __global float2* posBuffer, 
                              __global float4 *color,
                              __global float *fft,
                              const uint num_particles,
                              const uint num_nodes,
                              const float dt,
                              const float damping,
                              const float alpha,
                              const uint flags,
                              const float2 mousePos,
                              const float2 dimensions)
{
	const uint i = get_global_id(0);
    if( i >= num_particles )
    {
        return;
    }
    
    __global tParticle *particle = &particles[i];
    
    float2 a = (float2)(0.0f, 0.0f);
    
	float2 dist = mousePos - posBuffer[i];
    float invDistSq = 0.0f;
    if( flags & PARTICLE_FLAGS_INVSQR )
    {
        invDistSq = rsqrt(dist.x*dist.x + dist.y*dist.y);
        //invDistSq = invDistSq * invDistSq;
    }
    else
    {
        invDistSq = 1.0f / dot(dist, dist);
    }
	a += dist * MOUSE_FORCE * invDistSq;
    
	particle->mVel += /*(dimensions*0.5f - posBuffer[i]) * CENTER_FORCE -*/ particle->mMass * a;
	
	float speed2 = dot(particle->mVel, particle->mVel);
	if(speed2 < MIN_SPEED)
    {
        posBuffer[i] = mousePos + dist * (1 + particle->mMass);
    }
    
	posBuffer[i] += particle->mVel * dt;
	particle->mVel *= damping;

    // color
    if( flags & PARTICLE_FLAGS_SHOW_DARK )
    {
        color[i].x = 1.0f;
        color[i].y = 1.0f;
        color[i].z = 1.0f;
        color[i].w = alpha;
    }
    else if( flags & PARTICLE_FLAGS_SHOW_SPEED )
    {
        float speed = 10.0f*rsqrt(particle->mVel.x*particle->mVel.x + particle->mVel.y*particle->mVel.y);
        color[i].y = speed;
        color[i].z = speed;
        color[i].w = alpha;
    }
    else if( flags & PARTICLE_FLAGS_SHOW_MASS )
    {
        const int fftIdx = i%512;
        float massRatio = particle->mMass / MAX_MASS;
        color[i].x = massRatio*massRatio + (fft[fftIdx]/2.0f);
        color[i].y = massRatio/2.0f + (fftIdx/512);
        color[i].z = massRatio - fft[fftIdx];
        color[i].w = alpha + fft[fftIdx];
    }
}
