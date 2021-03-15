#version 330 compatibility

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices=5) out;

flat out int   f_patchid;
     out vec3  f_pos;
flat out vec3  f_norm;

uniform mat4 camera;
uniform int patchofs;
uniform mat4 frame;


void emit(int idx, vec3 pt) {
  f_pos = pt;
  const vec2 quad[] = {
    vec2(0,0),
    vec2(16,0),
    vec2(16,16),
    vec2(0,16)
  };
  vec2 uv = (quad[idx] + 16*vec2(gl_PrimitiveIDIn%64, gl_PrimitiveIDIn/64)) / 1024.0;
  gl_Position = vec4(uv * 2.0 - 1.0, 0, 1);
  EmitVertex();
}

void main() {
  vec3 pts[] = {
    vec3(frame*gl_in[0].gl_Position),
    vec3(frame*gl_in[1].gl_Position),
    vec3(frame*gl_in[2].gl_Position),
    vec3(frame*gl_in[3].gl_Position),
  };
  vec3 a = pts[1] - pts[0];
  vec3 b = pts[3] - pts[0];
  f_norm = normalize(cross(a, b));
  f_patchid = gl_PrimitiveIDIn + patchofs;

  emit(0, pts[0]); emit(1, pts[1]); emit(3, pts[3]); emit(2, pts[2]);
  EndPrimitive();
}
