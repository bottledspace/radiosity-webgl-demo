#version 330 compatibility

layout(location = 0) in vec4 pos_in;
layout(location = 1) in vec2 uv_in;
layout(location = 2) in int id_in;
layout(location = 3) in vec3 norm_in;
layout(location = 4) in float area_in;

flat out int   f_id;
flat out float f_depth;

uniform sampler2D resid;

void main() {
    int k = id_in%(64*64);
    ivec2 texofs = ivec2(k%64, k/64);
    float energy = texelFetch(resid, texofs, 4).r;
    f_depth = 1.0/(area_in*energy+1);
    gl_Position = vec4(uv_in * 2.0 - 1.0, 0, 1.0);
    f_id = id_in+1;
}