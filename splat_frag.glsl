#version 330 compatibility

     in vec3  f_pos;
flat in int   f_id;
flat in vec3  f_norm;

out layout(location=0) vec3 accum;
out layout(location=1) vec3 resid;

uniform mat4       views[5];
uniform int        emitter_id;
uniform float      energy;
uniform float      darea;
uniform mat4       frame;
uniform isampler2D hcube;
uniform sampler2D  prev_accum;
uniform sampler2D  prev_resid;

int which_side(vec3 p) {
  if (p.z < 0) {
    if (abs(p.y) < abs(p.z) && abs(p.x) < abs(p.z))
      return 0; // forward
    else if (p.x < p.y) {
      if (p.x < -p.y)
        return 3; // down
      else
        return 2; // right
    } else {
      if (p.x < -p.y)
        return 1; // left
      else
        return 4; // up
    }
  }
  else return -1;
}

void main() {
    accum = texelFetch(prev_accum, ivec2(gl_FragCoord.xy), 0).rgb;
    resid = texelFetch(prev_resid, ivec2(gl_FragCoord.xy), 0).rgb;

    if (f_id == emitter_id) {
      // Clear the residual energy of emitter
      resid = vec3(0,0,0);
      return;
    }
    int side = which_side(f_pos);
    if (side < 0)
      return;   // No energy transferred behind emitter
    
    // Project patch location onto hemicube
    vec4 projected = views[side] * vec4(f_pos,1);
    vec2 uv = clamp(projected.xy / projected.w, -1, 1);
    uv = (uv + vec2(1 + 2*(side%4),1 + 2*(side/4))) * 512;
    if (texture(hcube, (uv)/vec2(4,2)/1024.0, 0).r != f_id)
      return;  // Not visible to emitter

    // Patch location is relative to emitter
    vec3 r = normalize(f_pos);
    float Fij = max(dot(f_norm, r) * -dot(vec3(0,0,-1), r), 0)
      / (3.14159 * dot(f_pos, f_pos));

    accum += 0.5*energy*darea*vec3(Fij,0,0);
    resid += 0.5*energy*darea*vec3(Fij,0,0);
}