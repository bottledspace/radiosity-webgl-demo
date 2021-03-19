#version 300 es
precision highp float;

in vec2 uv;

out vec3 color;

uniform sampler2D resid;
uniform float       emitter_id;


void main() {
    color = texelFetch(resid, ivec2(uv), 0).rgb;
}
