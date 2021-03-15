#version 330 compatibility

layout (location = 0) in vec4 pos_in;
out int v_patchid;

void main() {
    gl_Position = pos_in;
}