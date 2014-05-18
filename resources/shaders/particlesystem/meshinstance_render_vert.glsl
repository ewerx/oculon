#version 120
#extension GL_ARB_draw_instanced	: enable
#extension GL_EXT_gpu_shader4		: enable

uniform sampler2D posMap;
//uniform sampler2D velocityMap;
//uniform sampler2D intensityMap;

//uniform mat4 MV;                //modelview matrix
//uniform mat4 P;                 //projection matrix
//uniform float spriteWidth;      //object space width of sprite (maybe an per-vertex in)
//uniform float texSize;
//
//uniform bool audioReactive;
//uniform float gain;

uniform float texSize;

varying float instanceID;
//varying vec4 position;

//varying vec4 color;

void main()
{
    instanceID      = float( gl_InstanceID );
    
    // get texture coords for instance
    vec2 uv;
    uv.x = float( mod( instanceID, texSize ) ) / texSize;
    uv.y = float( floor( instanceID / texSize ) ) / texSize;
    
    vec3 instancePos = texture2D( posMap, uv ).xyz;
    vec4 vertexPos = gl_Vertex;
    
    vertexPos.xyz += instancePos;
    
    gl_Position = gl_ModelViewProjectionMatrix * vertexPos;
    
//    color = vec4(1.0,1.0,1.0,0.0);
//    if (audioReactive)
//    {
//        // alpha from audio texture
//        color.a = texture2D( intensityMap, vec2(gl_MultiTexCoord0.s,0.0) ).x * gain;
//    }
//    else
//    {
//        color.a = 1.0;
//    }
    
    // dyanmic point size
//    vec4 eyePos = MV * vec4(pos.x, pos.y, pos.z, 1);
//    vec4 projCorner = P * vec4(0.5*spriteWidth, 0.5*spriteWidth, eyePos.z, eyePos.w);
//    gl_PointSize = screenWidth * projCorner.x / projCorner.w;
//    // uncomment for static point size
//    //	gl_Position = gl_ModelViewProjectionMatrix * vec4( pos.xyz, 1.0) ;
//    //  gl_PointSize = 4.0;
    
//    gl_Position = P * eyePos;
//    gl_TexCoord[0] = gl_MultiTexCoord0;
//	gl_FrontColor  = gl_Color;
}
