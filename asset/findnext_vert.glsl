#version 300 es
precision highp float;
layout (location = 0) in vec4 pos_in;

void main() {
    gl_Position = pos_in;
}