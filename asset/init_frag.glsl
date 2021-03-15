#version 300 es
precision highp float;
out layout(location=0) vec3 accum;
out layout(location=1) vec3 resid;

uniform sampler2D transfer;

void main() {
    resid = texelFetch(transfer, ivec2(gl_FragCoord.xy), 0).rgb;
}