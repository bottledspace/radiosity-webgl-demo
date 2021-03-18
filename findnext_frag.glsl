#version 330 compatibility

layout(location = 0) out int res_id;

flat in int   f_id;
flat in float f_depth;


void main() {
    gl_FragDepth = f_depth;
    res_id = f_id;
}