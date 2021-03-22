#version 300 es
precision highp float;

     in vec3  f_pos;
flat in float f_id;
flat in vec3  f_norm;

layout(location=0) out vec3 accum;
layout(location=1) out vec3 resid;

uniform mat4       views[5];
uniform float      emitter_id;
uniform vec3       energy;
uniform float      darea;
uniform highp sampler2D  hcube[5];
uniform highp sampler2D  prev_accum;
uniform highp sampler2D  prev_resid;

int which_side(vec3 p) {
  if (p.z < 0.0) {
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
    vec2 uv = (clamp(projected.xy / projected.w, -1.0, 1.0) + 1.0) / 2.0;
	// We need to go through each side like this because dereferencing the
	// sampler is only allowed with a constant.
    if ((side == 0 && abs(texture(hcube[0], uv).r - f_id) >= 0.1)
	 || (side == 1 && abs(texture(hcube[1], uv).r - f_id) >= 0.1)
	 || (side == 2 && abs(texture(hcube[2], uv).r - f_id) >= 0.1)
	 || (side == 3 && abs(texture(hcube[3], uv).r - f_id) >= 0.1)
	 || (side == 4 && abs(texture(hcube[4], uv).r - f_id) >= 0.1))
      return;  // Not visible to emitter

    // Patch location is relative to emitter
    vec3 r = normalize(f_pos);
    float Fij = max(dot(f_norm, r) * dot(vec3(0,0,1), r), 0.0)
      / (3.14159 * dot(f_pos, f_pos));

    accum += 0.5*energy*darea*Fij;
    resid += 0.5*energy*darea*Fij;
}