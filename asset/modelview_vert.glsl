#version 300 es
precision highp float;
layout (location = 0) in vec4 pos_in;

uniform mat4 mvp;


void main() {
    gl_Position = mvp * pos_in;
}