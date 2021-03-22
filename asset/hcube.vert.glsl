#version 300 es
precision highp float;

layout(location = 0) in vec4 loc_in;
layout(location = 1) in vec2 uv_in;
layout(location = 2) in float id_in;
layout(location = 3) in vec3 norm_in;
layout(location = 4) in float area_in;

flat out float f_id;

uniform int side;
uniform vec4 views[20];
uniform vec4 frame[4];

void main() {
    gl_Position = mat4(views[4*side],views[4*side+1],
		views[4*side+2],views[4*side+3]) * mat4(frame[0],frame[1],
		frame[2],frame[3]) * loc_in;
    f_id = id_in;
}