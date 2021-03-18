#version 330 compatibility

layout (location = 0) in vec4 loc_in;
layout (location = 1) in vec2 uv_in;
layout (location = 2) in int id_in;

uniform mat4 mvp;


void main() {
    int k = id_in%(64*64);
    gl_Position = mvp * loc_in;
    gl_TexCoord[0].xy = (uv_in + vec2(k%64, k/64))*16;
}