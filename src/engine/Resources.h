#pragma once
#include "cinder/CinderResources.h"

#define RES_PARTICLE                CINDER_RESOURCE( ../resources/images/, particle.png, 128, PNG )
#define RES_EMITTER                 CINDER_RESOURCE( ../resources/, emitter.png, 129, PNG )
#define RES_NORMAL                  CINDER_RESOURCE( ../resources/, normal.png, 130, PNG )
#define RES_BUMP                    CINDER_RESOURCE( ../resources/, bump.png, 131, PNG )
#define RES_SPEC_EXPONENT           CINDER_RESOURCE( ../resources/, specExponent.png, 132, PNG )
#define RES_SHADER_PASSTHRU			CINDER_RESOURCE( ../resources/shaders/, passthru.vsh, 133, GLSL )
#define RES_SHADER_FRAGMENT			CINDER_RESOURCE( ../resources/shaders/, shader.fsh, 134, GLSL )
#define RES_EMITTER_VERT			CINDER_RESOURCE( ../resources/shaders/, emitter_vert.glsl, 135, GLSL )
#define RES_EMITTER_FRAG			CINDER_RESOURCE( ../resources/shaders/, emitter_frag.glsl, 136, GLSL )