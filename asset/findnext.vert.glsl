#version 300 es
precision highp float;
precision highp int;

layout(location = 0) in vec4 pos_in;
layout(location = 1) in vec2 uv_in;
layout(location = 2) in float id_in;
layout(location = 3) in vec3 norm_in;
layout(location = 4) in float area_in;

flat out float f_id;
flat out vec3 f_energy;
flat out float f_notused;

uniform sampler2D resid;

void main() {
    int k = int(id_in)%(64*64);
    ivec2 texofs = ivec2(k%64, k/64);
	
	// Calculate the average energy of the patch.
	vec3 energy = vec3(0.0,0.0,0.0);
	for (int i = 0; i < 16; i++)
	for (int j = 0; j < 16; j++) {
		energy += texelFetch(resid, texofs*16+ivec2(i,j), 0).xyz;
	}
	energy = area_in * energy / (16.0*16.0);
	
	float depth = 1.0/(dot(energy,energy)+1.0);
    gl_Position = vec4(uv_in * 2.0 - 1.0, depth, 1.0);
	
    f_id = float(int(id_in)+1);
	f_energy = energy;
	f_notused = pos_in.x;
}