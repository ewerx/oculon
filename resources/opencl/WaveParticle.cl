typedef struct {
	float2 vel;
	float mass;
	float life;
} Particle;


typedef struct {
	float2 pos;
	float spread;
	float attractForce;
	float waveAmp;
	float waveFreq;
} Node;

#define kMaxParticles		512*512

#define kArg_particles			0
#define kArg_posBuffer			1
#define kArg_colBuffer			2
#define kArg_nodes				3
#define kArg_numNodes			4
#define kArg_color				5
#define kArg_colorTaper			6
#define kArg_momentum			7
#define kArg_dieSpeed			8
#define kArg_time				9
#define kArg_wavePosMult		10
#define kArg_waveVelMult		11
#define kArg_massMin			12


float rand(float2 co) {
	float i;
	return fabs(fract(sin(dot(co.xy ,make_float2(12.9898f, 78.233f))) * 43758.5453f, &i));
}


__kernel void updateParticle(__global Particle* particles,		//0
                             __global float2* posBuffer,		//1
                             __global float4 *colBuffer,		//2
                             __global Node *nodes,				//3
                             const int numNodes,				//4
                             const float4 color,				//5
                             const float colorTaper,			//6
                             const float momentum,				//7
                             const float dieSpeed,				//8
                             const float time,					//9
                             const float wavePosMult,			//10
                             const float waveVelMult,			//11
                             const float massMin				//12
                             ) {				
	
	int		id					= get_global_id(0);
	__global Particle	*p		= &particles[id];
	float2	pos					= posBuffer[id];
	
	int		birthNodeId			= id % numNodes;
	float2	vecFromBirthNode	= pos - nodes[birthNodeId].pos;							// vector from birth node to particle
	float	distToBirthNode		= fast_length(vecFromBirthNode);							// distance from bith node to particle
	
	int		targetNodeId		= (id % 2 == 0) ? (id+1) % numNodes : (id + numNodes-1) % numNodes;
	float2	vecFromTargetNode	= pos - nodes[targetNodeId].pos;						// vector from target node to particle
	float	distToTargetNode	= fast_length(vecFromTargetNode);						// distance from target node to particle
	
	float2	diffBetweenNodes	= nodes[targetNodeId].pos - nodes[birthNodeId].pos;		// vector between nodes (from birth to target)
	float2	normBetweenNodes	= fast_normalize(diffBetweenNodes);						// normalized vector between nodes (from birth to target)
	float	distBetweenNodes	= fast_length(diffBetweenNodes);						// distance betweem nodes (from birth to target)

	float	dotTargetNode		= fmax(0.0f, dot(vecFromTargetNode, -normBetweenNodes));
	float	dotBirthNode		= fmax(0.0f, dot(vecFromBirthNode, normBetweenNodes));
	float	distRatio			= fmin(1.0f, fmin(dotTargetNode, dotBirthNode) / (distBetweenNodes * 0.5f));
	
	// add attraction to other nodes
	p->vel						-= vecFromTargetNode * nodes[targetNodeId].attractForce / (distToTargetNode + 1.0f) * p->mass;

	// add wave
	float2 waveVel				= make_float2(-normBetweenNodes.y, normBetweenNodes.x) * sin(time + 10.0f * 3.1416926f * distRatio * nodes[birthNodeId].waveFreq);
	float2 sideways				= nodes[birthNodeId].waveAmp * waveVel * distRatio * p->mass;
	posBuffer[id]				+= sideways * wavePosMult;
	p->vel						+= sideways * waveVelMult * dotTargetNode / (distBetweenNodes + 1);
	
	// set color
	float invLife = 1.0f - p->life;
	colBuffer[id] = color * (1.0f - invLife * invLife * invLife);// * sqrt(p->life);	// fade with life
	
	// add waviness
	p->life -= dieSpeed;
	if(p->life < 0.0f || distToTargetNode < 1.0f) {
		posBuffer[id] = posBuffer[id + kMaxParticles] = nodes[birthNodeId].pos;
		float a = rand(p->vel) * 3.1415926f * 30.0f;
		float r = rand(pos);
		p->vel = make_float2(cos(a), sin(a)) * (nodes[birthNodeId].spread * r * r * r);
		p->life = 1.0f;
//		p->mass = mix(massMin, 1.0f, r);
	} else {
		posBuffer[id+kMaxParticles] = pos;
		colBuffer[id+kMaxParticles] = colBuffer[id] * (1.0f - colorTaper);	
		
		posBuffer[id] += p->vel;
		p->vel *= momentum;
	}
}
