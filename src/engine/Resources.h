#pragma once
#include "cinder/CinderResources.h"

// textures
#define RES_PARTICLE                CINDER_RESOURCE( ../resources/images/, particle.png, 128, PNG )
#define RES_PARTICLE_WHITE          CINDER_RESOURCE( ../resources/images/, particle_white.png, 128, PNG )
#define RES_GLITTER                 CINDER_RESOURCE( ../resources/images/, glitter.png, 128, PNG )
#define RES_EMITTER                 CINDER_RESOURCE( ../resources/, emitter.png, 129, PNG )
#define RES_NORMAL                  CINDER_RESOURCE( ../resources/, normal.png, 130, PNG )
#define RES_BUMP                    CINDER_RESOURCE( ../resources/, bump.png, 131, PNG )
#define RES_SPEC_EXPONENT           CINDER_RESOURCE( ../resources/, specExponent.png, 132, PNG )

// shaders
#define RES_SHADER_PASSTHRU			CINDER_RESOURCE( ../resources/shaders/, passthru.vsh, 133, GLSL )
#define RES_SHADER_FRAGMENT			CINDER_RESOURCE( ../resources/shaders/, shader.fsh, 134, GLSL )
#define RES_EMITTER_VERT			CINDER_RESOURCE( ../resources/shaders/, emitter_vert.glsl, 135, GLSL )
#define RES_EMITTER_FRAG			CINDER_RESOURCE( ../resources/shaders/, emitter_frag.glsl, 136, GLSL )
#define RES_BLUR2_VERT              CINDER_RESOURCE( ../resources/shaders/, blur2_vert.glsl, 135, GLSL )
#define RES_BLUR2_FRAG              CINDER_RESOURCE( ../resources/shaders/, blur2_frag.glsl, 136, GLSL )

#define RES_PASSTHRU_VERT           CINDER_RESOURCE( ../resources/shaders/, passThru_vert.glsl, 128, GLSL )
#define RES_BLUR_FRAG               CINDER_RESOURCE( ../resources/shaders/, gaussianBlur_frag.glsl, 129, GLSL )

#define RES_MOTIONBLUR_FRAG         CINDER_RESOURCE( ../resources/shaders/, motionBlur.frag, 129, GLSL )

// orbiter textures
#define RES_ORBITER_SUN             CINDER_RESOURCE( ../resources/images/orbiter/, sunmap.jpg, 200, JPG )
#define RES_ORBITER_MERCURY         CINDER_RESOURCE( ../resources/images/orbiter/, mercurymap.jpg, 201, JPG )
#define RES_ORBITER_VENUS           CINDER_RESOURCE( ../resources/images/orbiter/, venusmap.jpg, 202, JPG )
#define RES_ORBITER_EARTH           CINDER_RESOURCE( ../resources/images/orbiter/, earthmap1k.jpg, 203, JPG )
#define RES_ORBITER_MARS            CINDER_RESOURCE( ../resources/images/orbiter/, marsmap1k.jpg, 204, JPG )
#define RES_ORBITER_JUPITER         CINDER_RESOURCE( ../resources/images/orbiter/, jupitermap.jpg, 205, JPG )
#define RES_ORBITER_SATURN          CINDER_RESOURCE( ../resources/images/orbiter/, saturnmap.jpg, 206, JPG )
#define RES_ORBITER_URANUS          CINDER_RESOURCE( ../resources/images/orbiter/, uranusmap.jpg, 207, JPG )
#define RES_ORBITER_NEPTUNE         CINDER_RESOURCE( ../resources/images/orbiter/, neptunemap.jpg, 208, JPG )