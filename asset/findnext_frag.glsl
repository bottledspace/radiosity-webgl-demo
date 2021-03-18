#version 300 es
precision highp float;

layout(location = 0) out vec3 color;

flat in int   f_id;
flat in float f_depth;

void main() {
    gl_FragDepth = f_depth;
    color.r = float(f_id);
	color.g = f_depth;
}