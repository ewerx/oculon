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
#define RES_FISHEYE_VERT			CINDER_RESOURCE( ../resources/shaders/, fisheye_vert.glsl, 136, GLSL )

#define RES_PASSTHRU_VERT           CINDER_RESOURCE( ../resources/shaders/, passThru_vert.glsl, 128, GLSL )
#define RES_BLUR_FRAG               CINDER_RESOURCE( ../resources/shaders/, gaussianBlur_frag.glsl, 129, GLSL )

#define RES_MOTIONBLUR_FRAG         CINDER_RESOURCE( ../resources/shaders/, motionBlur.frag, 129, GLSL )
#define RES_PASSTHRU2_VERT          CINDER_RESOURCE( ../resources/shaders/, passThru.vert, 129, GLSL )

// shader test
#define RES_SHADER_SIMPLICITY_FRAG	CINDER_RESOURCE( ../resources/shaders/, simplicity_frag.glsl, 134, GLSL )
#define RES_SHADER_MENGER_FRAG	CINDER_RESOURCE( ../resources/shaders/, menger_frag.glsl, 134, GLSL )
#define RES_SHADER_PIXELATE_FRAG	CINDER_RESOURCE( ../resources/shaders/, pixelate_frag.glsl, 134, GLSL )
#define RES_SHADER_PAINT_FRAG	CINDER_RESOURCE( ../resources/shaders/, kifs_frag.glsl, 134, GLSL )
#define RES_SHADER_POLYCHORA_FRAG	CINDER_RESOURCE( ../resources/shaders/, polychora_frag.glsl, 134, GLSL )

#define RES_COLORTEX1		CINDER_RESOURCE( ../resources/images/, colortex1.jpg,		138, IMAGE )
#define RES_COLORTEX2		CINDER_RESOURCE( ../resources/images/, colortex2.jpg,		138, IMAGE )
#define RES_COLORTEX3		CINDER_RESOURCE( ../resources/images/, colortex3.jpg,		138, IMAGE )
#define RES_COLORTEX4		CINDER_RESOURCE( ../resources/images/, colortex4.jpg,		138, IMAGE )

// tectonic
#define RES_EARTH_FRAG		CINDER_RESOURCE( ../resources/shaders/, earth_frag.glsl, 128, GLSL )
#define RES_EARTHDIFFUSE	CINDER_RESOURCE( ../resources/images/tectonic/, earthDiffuse.png, 129, PNG )
#define RES_EARTHMASK		CINDER_RESOURCE( ../resources/images/tectonic/, earthMask.png, 130, PNG )
#define RES_EARTHNORMAL		CINDER_RESOURCE( ../resources/images/tectonic/, earthNormal.png, 131, PNG )
#define RES_QUAKE_FRAG		CINDER_RESOURCE( ../resources/shaders/, quake_frag.glsl, 133, GLSL )
#define RES_QUAKE_VERT		CINDER_RESOURCE( ../resources/shaders/, quake_vert.glsl, 134, GLSL )

// sol
#define RES_SOLMASK         CINDER_RESOURCE( ../resources/images/sol/, solmask.png, 129, PNG )

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

// catalog
#define RES_BRIGHT_STARS_VERT		CINDER_RESOURCE( ../resources/shaders/, brightStars.vert, 100, GLSL )
#define RES_BRIGHT_STARS_FRAG		CINDER_RESOURCE( ../resources/shaders/, brightStars.frag, 100, GLSL )
#define RES_FAINT_STARS_VERT		CINDER_RESOURCE( ../resources/shaders/, faintStars.vert, 100, GLSL )
#define RES_FAINT_STARS_FRAG		CINDER_RESOURCE( ../resources/shaders/, faintStars.frag, 100, GLSL )

#define RES_CATALOG_MILKYWAY        CINDER_RESOURCE( ../resources/images/catalog/, milkyWay.jpg, 200, JPG )
#define RES_CATALOG_STAR            CINDER_RESOURCE( ../resources/images/catalog/, star.png, 200, PNG )
#define RES_CATALOG_STARGLOW        CINDER_RESOURCE( ../resources/images/catalog/, starGlow.png, 200, PNG )
#define RES_CATALOG_DARKSTAR        CINDER_RESOURCE( ../resources/images/catalog/, darkStar.png, 200, PNG )
#define RES_CATALOG_SPECTRUM        CINDER_RESOURCE( ../resources/images/catalog/, spectrum.png, 200, PNG )

// flock
#define RES_FLOCK_CUBE1_ID		CINDER_RESOURCE( ../resources/, cubePosX.jpg,		138, IMAGE )
#define RES_FLOCK_CUBE2_ID		CINDER_RESOURCE( ../resources/, cubePosY.jpg,		139, IMAGE )
#define RES_FLOCK_CUBE3_ID		CINDER_RESOURCE( ../resources/, cubePosZ.jpg,		140, IMAGE )
#define RES_FLOCK_CUBE4_ID		CINDER_RESOURCE( ../resources/, cubeNegX.jpg,		141, IMAGE )
#define RES_FLOCK_CUBE5_ID		CINDER_RESOURCE( ../resources/, cubeNegY.jpg,		142, IMAGE )
#define RES_FLOCK_CUBE6_ID		CINDER_RESOURCE( ../resources/, cubeNegZ.jpg,		143, IMAGE )
#define RES_FLOCK_LANTERNGLOW_PNG	CINDER_RESOURCE( ../resources/, lanternGlow.png,	150, IMAGE )
#define RES_FLOCK_GLOW_PNG		CINDER_RESOURCE( ../resources/, glow.png,			151, IMAGE )
#define RES_FLOCK_NEBULA_PNG		CINDER_RESOURCE( ../resources/, nebula.png,			152, IMAGE )
#define RES_FLOCK_PASSTHRU_VERT	CINDER_RESOURCE( ../resources/, passThruFlock.vert,		160, GLSL )
#define RES_FLOCK_VELOCITY_FRAG	CINDER_RESOURCE( ../resources/, Velocity.frag,		161, GLSL )
#define RES_FLOCK_POSITION_FRAG	CINDER_RESOURCE( ../resources/, Position.frag,		162, GLSL )
#define RES_FLOCK_P_VELOCITY_FRAG CINDER_RESOURCE( ../resources/, P_Velocity.frag,	163, GLSL )
#define RES_FLOCK_P_POSITION_FRAG CINDER_RESOURCE( ../resources/, P_Position.frag,	164, GLSL )
#define RES_FLOCK_LANTERN_VERT	CINDER_RESOURCE( ../resources/, lantern.vert,		165, GLSL )
#define RES_FLOCK_LANTERN_FRAG	CINDER_RESOURCE( ../resources/, lantern.frag,		166, GLSL )
#define RES_FLOCK_ROOM_VERT		CINDER_RESOURCE( ../resources/, room.vert,			167, GLSL )
#define RES_FLOCK_ROOM_FRAG		CINDER_RESOURCE( ../resources/, room.frag,			168, GLSL )
#define RES_FLOCK_VBOPOS_VERT		CINDER_RESOURCE( ../resources/, VboPos.vert,		169, GLSL )
#define RES_FLOCK_VBOPOS_FRAG		CINDER_RESOURCE( ../resources/, VboPos.frag,		170, GLSL )
#define RES_FLOCK_P_VBOPOS_VERT	CINDER_RESOURCE( ../resources/, P_VboPos.vert,		171, GLSL )
#define RES_FLOCK_P_VBOPOS_FRAG	CINDER_RESOURCE( ../resources/, P_VboPos.frag,		172, GLSL )
#define RES_FLOCK_GLOW_FRAG		CINDER_RESOURCE( ../resources/, glow.frag,			173, GLSL )
#define RES_FLOCK_NEBULA_FRAG		CINDER_RESOURCE( ../resources/, nebula.frag,		174, GLSL )

// corona
#define RES_CUBE1_ID		CINDER_RESOURCE( ../resources/, cubePosX.jpg,		138, IMAGE )
#define RES_CUBE2_ID		CINDER_RESOURCE( ../resources/, cubePosY.jpg,		139, IMAGE )
#define RES_CUBE3_ID		CINDER_RESOURCE( ../resources/, cubePosZ.jpg,		140, IMAGE )
#define RES_CUBE4_ID		CINDER_RESOURCE( ../resources/, cubeNegX.jpg,		141, IMAGE )
#define RES_CUBE5_ID		CINDER_RESOURCE( ../resources/, cubeNegY.jpg,		142, IMAGE )
#define RES_CUBE6_ID		CINDER_RESOURCE( ../resources/, cubeNegZ.jpg,		143, IMAGE )

// terrain
#define RES_TERRAIN_FRAG    CINDER_RESOURCE( ../resources/terrain/, terrain_shader_frag.glsl,		160, GLSL )
#define RES_TERRAIN_VERT    CINDER_RESOURCE( ../resources/terrain/, terrain_shader_vert.glsl,		160, GLSL )
#define RES_TERRAIN_TEXTURE CINDER_RESOURCE( ../resources/terrain/, terrain_texture.jpg,			151, IMAGE )
#define RES_SHADER_TEX_FRAG		CINDER_RESOURCE( ../resources/terrain/, terrain_tex_frag.glsl,	128, GLSL	)
#define RES_SHADER_TEX_VERT		CINDER_RESOURCE( ../resources/terrain/, terrain_tex_vert.glsl,	129, GLSL	)
#define RES_SHADER_VTF_FRAG		CINDER_RESOURCE( ../resources/terrain/, terrain_vtf_frag.glsl,	130, GLSL	)
#define RES_SHADER_VTF_VERT		CINDER_RESOURCE( ../resources/terrain/, terrain_vtf_vert.glsl,	131, GLSL	)

// contour
#define RES_SHADER_CT_TEX_FRAG		CINDER_RESOURCE( ../resources/contour/, contour_tex_frag.glsl,	128, GLSL	)
#define RES_SHADER_CT_TEX_VERT		CINDER_RESOURCE( ../resources/contour/, contour_tex_vert.glsl,	129, GLSL	)

// objects
#define RES_CUBE_OBJ		CINDER_RESOURCE( ../resources/, cube.obj, 128, DATA )

// polyhedron
#define RES_POLYHEDRON_TEX1		CINDER_RESOURCE( ../resources/images/, redgreenyellow.png,		138, IMAGE )
#define RES_POLYHEDRON_SHADER_FRAG		CINDER_RESOURCE( ../resources/shaders/polyhedron/, polyhedron_frag.glsl, 128, GLSL )
#define RES_POLYHEDRON_SHADER_VERT		CINDER_RESOURCE( ../resources/shaders/polyhedron/, polyhedron_vert.glsl, 129, GLSL )