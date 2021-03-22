#version 300 es
precision highp float;

layout (location = 0) in vec3 loc_in;
layout (location = 1) in vec2 uv_in;
layout (location = 2) in float id_in;

out vec2 uv;

uniform vec4 mvp[4];


void main() {
    int k = int(id_in)%(64*64);
    gl_Position = mat4(mvp[0],mvp[1],mvp[2],mvp[3]) * vec4(loc_in, 1.0);
    uv = (uv_in + vec2(k%64, k/64))*16.0;
}