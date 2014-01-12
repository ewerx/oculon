// http://strattonbrazil.blogspot.ca/2011/09/single-pass-wireframe-rendering_10.html

#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_EXT_geometry_shader4 : enable
varying in vec3 vertWorldPos[3];
varying in vec3 vertWorldNormal[3];
varying out vec3 worldNormal;
varying out vec3 worldPos;
uniform vec2 WIN_SCALE;
noperspective varying vec3 dist;

void main(void)
{
    // taken from 'Single-Pass Wireframe Rendering'
    vec2 p0 = WIN_SCALE * gl_PositionIn[0].xy/gl_PositionIn[0].w;
    vec2 p1 = WIN_SCALE * gl_PositionIn[1].xy/gl_PositionIn[1].w;
    vec2 p2 = WIN_SCALE * gl_PositionIn[2].xy/gl_PositionIn[2].w;
    vec2 v0 = p2-p1;
    vec2 v1 = p2-p0;
    vec2 v2 = p1-p0;
    float area = abs(v1.x*v2.y - v1.y * v2.x);
    
    dist = vec3(area/length(v0),0,0);
    worldPos = vertWorldPos[0];
    worldNormal = vertWorldNormal[0];
    gl_Position = gl_PositionIn[0];
    EmitVertex();
    dist = vec3(0,area/length(v1),0);
    worldPos = vertWorldPos[1];
    worldNormal = vertWorldNormal[1];
    gl_Position = gl_PositionIn[1];
    EmitVertex();
    dist = vec3(0,0,area/length(v2));
    worldPos = vertWorldPos[2];
    worldNormal = vertWorldNormal[2];
    gl_Position = gl_PositionIn[2];
    EmitVertex();
    EndPrimitive();
}