#version 300 es
precision highp float;

layout (location = 0) in vec4 loc_in;
layout (location = 1) in vec2 uv_in;
layout (location = 2) in float id_in;

out vec2 uv;

uniform mat4 mvp;


void main() {
    int k = int(id_in)%(64*64);
    gl_Position = mvp * loc_in;
    uv = (uv_in + vec2(k%64, k/64))*16.0;
}