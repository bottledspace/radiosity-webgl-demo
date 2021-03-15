#version 330 compatibility

layout (location = 0) in vec4 pos_in;

void main() {
    gl_Position = pos_in;
}