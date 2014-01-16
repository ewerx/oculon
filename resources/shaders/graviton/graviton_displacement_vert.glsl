#version 120
#extension GL_ARB_point_parameters : enable
#extension GL_ARB_point_sprite : enable

uniform sampler2D displacementMap;
uniform sampler2D velocityMap;
uniform sampler2D intensityMap;

uniform mat4 MV;                //modelview matrix
uniform mat4 P;                 //projection matrix
uniform float spriteWidth;      //object space width of sprite (maybe an per-vertex in)
uniform float screenWidth;      //screen width in pixels
uniform float gain;

varying vec4 color;

void main()
{
    //using the displacement map to move vertices
	vec4 pos = texture2D( displacementMap, gl_MultiTexCoord0.xy );
    //color = texture2D( velocityMap, gl_MultiTexCoord0.xy );
    color = vec4(1.0,1.0,1.0,0.0);
    // alpha from audio texture
    color.a = texture2D( intensityMap, vec2(gl_MultiTexCoord0.s,0.0) ).x * gain;
    
    // uncomment to disable dynamic point size
//	gl_Position = gl_ModelViewProjectionMatrix * vec4( pos.xyz, 1.0) ;
//    gl_PointSize = 4.0;
    
    vec4 eyePos = MV * vec4(pos.x, pos.y, pos.z, 1);
    vec4 projCorner = P * vec4(0.5*spriteWidth, 0.5*spriteWidth, eyePos.z, eyePos.w);
    gl_PointSize = screenWidth * projCorner.x / projCorner.w;
    gl_Position = P * eyePos;
}