#version 330 compatibility

layout (location = 0) in vec4 pos_in;

uniform mat4 mvp;


void main() {
    gl_Position = mvp * pos_in;
}