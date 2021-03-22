#version 300 es
precision highp float;
precision highp int;

layout(location = 0) out vec4 color;

flat in float f_id;
flat in vec3 f_energy;
flat in float f_notused;

void main() {
    color = vec4(f_energy, f_id);
}