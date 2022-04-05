#version 430

//good lord i hate my life. i have slept for maybe 3 of the last 36 hours
layout (location = 0) in vec2 UV;
layout (location = 2) in vec3 norm;

layout (location = 0) out vec4 outputCol;

#include "../../fragments/frame_uniforms.glsl"
#include "../../fragments/multiple_point_lights.glsl"

uniform layout(binding = 0) sampler2D sceneView;
uniform float Diffuse;

float CalcRimIntensity(vec3 eyeVec){
	float rimInten = dot(eyeVec, norm);
	rimInten = 1.0 - rimInten;

	return rimInten;
}

void main(){
	vec3 eye = normalize(-gl_Position.xyz);

	float rimLightIntensity = CalcRimIntensity(eye);
	
	vec4 diffuseVec = vec4(1.0, 1.0, 0.8, 0.6);

	vec4 rimLight = vec4((rimLightIntensity * diffuseVec).xyz, diffuseVec.w);

	vec4 fragCol = texture(sceneView, UV);
	fragCol.a = rimLight.a;
	fragCol.rgb += rimLight.rgb;

	outputCol = fragCol;
}
