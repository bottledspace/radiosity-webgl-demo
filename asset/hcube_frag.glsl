#version 300 es
precision highp float;

flat in int f_id;

layout(location = 0) out int color;

void main() {
    color = f_id;
}