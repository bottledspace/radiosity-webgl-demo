#version 300 es
precision highp float;
precision highp int;

layout(location = 0) out vec4 color;

flat in float f_id;
flat in float f_energy;
flat in float f_notused;

void main() {
    color.r = f_id;
	color.g = f_energy;
	color.b = f_notused; // test value
	color.a = 1.0;
}