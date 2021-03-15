#version 330 compatibility

     in vec2  f_uv;
flat in int   f_patchid;

uniform sampler2D resid;
uniform int       emitter_id;


void main() {
    if (emitter_id == f_patchid)
        gl_FragColor.rgb = vec3(1,1,0);
    else
        gl_FragColor.rgb = texelFetch(resid, ivec2(f_uv), 0).rgb;
}