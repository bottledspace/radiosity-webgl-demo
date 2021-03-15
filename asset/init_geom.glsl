#version 300 es
precision highp float;
layout (lines_adjacency) in;
layout (triangle_strip, max_vertices=5) out;


void emit(int idx) {
  const vec2 quad[] = {
    vec2(-1,-1),
    vec2(1,-1),
    vec2(1,1),
    vec2(-1,1)
  };
  gl_Position = vec4(quad[idx], 0, 1);
  EmitVertex();
}

void main() {
  emit(0); emit(1); emit(3); emit(2);
  EndPrimitive();
}
