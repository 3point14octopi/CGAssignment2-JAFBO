#version 440

//very dumb and should only be used for rim
layout (location = 0) in vec3 inPos;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outPos;
layout (location = 2) out vec3 outNormal;

#include "../fragments/frame_uniforms.glsl"

void main() {
    mat4 clipToView = inverse(u_Projection);

    gl_Position = vec4(inPos, 1);
    outUV = (inPos.xy + 1) / 2;
    outViewDir = inPos;//(clipToView * vec4(inPos.xy, 0, 1)).xyz;

    outNormal = inNormal;
}