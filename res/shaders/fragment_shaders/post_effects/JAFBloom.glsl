//:steman:

#version 430

layout(location = 0) in vec2 inUV;

out vec4 fragColour;

uniform layout(binding = 0) sampler2D blurred_lights;
uniform layout(binding = 1) sampler2D no_blur;

vec3 GammaCorrect(vec3 bloomVec){
	return pow(bloomVec, vec3(1.0/2.2));
}

void main(){
	vec3 cummulation = texture(no_blur, inUV).rgb;
	vec3 bloom = texture(blurred_lights, inUV).rgb;
	cummulation += bloom;

	vec3 result = GammaCorrect(cummulation);

	fragColour = vec4(result, 1.0);
}