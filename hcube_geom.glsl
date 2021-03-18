#version 330 compatibility

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices=5) out;

flat out int f_patchid;

void emit(int idx) {
  f_patchid = gl_PrimitiveIDIn;
  gl_Position = gl_in[idx].gl_Position;
  EmitVertex();
}

void main() {
  emit(0); emit(1); emit(3); emit(2);
  EndPrimitive();
}
