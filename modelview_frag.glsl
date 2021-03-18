#version 330 compatibility

uniform sampler2D resid;
uniform int       emitter_id;


void main() {
    gl_FragColor.rgb = texelFetch(resid, ivec2(gl_TexCoord[0].xy), 0).rgb;
}