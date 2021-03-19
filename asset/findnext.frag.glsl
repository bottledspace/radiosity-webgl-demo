#version 300 es
precision highp float;
precision highp int;

layout(location = 0) out vec4 color;

flat in float f_id;
flat in float f_energy;

void main() {
    color.r = f_id;
	color.g = f_energy;
	color.b = 333.0; // test value
	color.a = 1.0;
}