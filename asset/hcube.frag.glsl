#version 300 es
precision highp float;

flat in float f_id;

layout(location = 0) out float color;

void main() {
    color = f_id;
}