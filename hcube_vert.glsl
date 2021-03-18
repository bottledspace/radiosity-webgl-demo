#version 330 compatibility

layout(location = 0) in vec4 loc_in;
layout(location = 1) in vec2 uv_in;
layout(location = 2) in int id_in;
layout(location = 3) in vec3 norm_in;
layout(location = 4) in float area_in;

flat out int f_id;

uniform int side;
uniform mat4 views[5];
uniform mat4 frame;

void main() {
    gl_Position = views[side] * frame * loc_in;
    f_id = id_in;
}