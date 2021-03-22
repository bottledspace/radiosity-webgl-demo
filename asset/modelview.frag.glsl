#version 300 es
precision highp float;

in vec2 uv;

layout (location = 0) out vec4 color;

uniform sampler2D resid;
uniform float     emitter_id;


void main() {
    color = vec4(texelFetch(resid, ivec2(uv), 0).rgb, 1.0);
}
