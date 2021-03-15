#version 300 es
precision highp float;
layout(location=0) out int res_id;

flat in int   f_patchid;
flat in float f_area;

uniform sampler2D resid;
uniform int patchofs;

// bijective map from [-inf,inf] -> [0,1]
float pack(float x) { return atan(x)/3.14159+0.5; }

void main() {
    ivec2 texofs = ivec2(f_patchid%64, f_patchid/64);
    float energy = texelFetch(resid, texofs, 4).r;
    gl_FragDepth = 1.0/(f_area*energy+1.0);
    res_id = f_patchid+patchofs+1;
}