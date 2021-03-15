#version 330 compatibility

layout (location = 0) in vec4 pos_in;
uniform int side;
uniform mat4 views[5];
uniform mat4 frame;

void main() {
    gl_Position = views[side] * frame * pos_in;
}