#version 300 es
precision highp float;
layout (lines_adjacency) in;
layout (triangle_strip, max_vertices=5) out;

flat out int   f_patchid;
     out vec2  f_uv;

uniform int patchofs;


void emit(int idx) {
  const vec2 quad[] = {
    vec2(0,0),
    vec2(16,0),
    vec2(16,16),
    vec2(0,16)
  };
  int k = gl_PrimitiveIDIn;
  f_uv = quad[idx] + 16*vec2(k%64, k/64);
  f_patchid = k + patchofs;
  gl_Position = gl_in[idx].gl_Position;
  EmitVertex();
}

void main() {
  emit(0); emit(1); emit(3); emit(2);
  EndPrimitive();
}
