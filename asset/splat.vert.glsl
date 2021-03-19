#version 300 es
precision highp float;

layout(location = 0) in vec4 pos_in;
layout(location = 1) in vec2 uv_in;
layout(location = 2) in float id_in;
layout(location = 3) in vec4 norm_in;
layout(location = 4) in float area_in;

flat out float f_id;
     out vec3 f_pos;
flat out vec3 f_norm;

uniform mat4 camera;
uniform mat4 frame;

void main() {
    f_id = (id_in);
    f_pos = vec3(frame * pos_in);
    f_norm = mat3(frame) * vec3(norm_in);
    int k = int(id_in)%(64*64);
    vec2 uv = 16.0 * (uv_in + vec2(k%64, k/64)) / 1024.0;
    gl_Position = vec4(uv * 2.0 - 1.0, 0, 1);
}