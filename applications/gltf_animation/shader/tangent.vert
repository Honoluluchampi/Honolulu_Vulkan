#version 450

#extension GL_GOOGLE_include_directive     : enable
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

// INPUT
layout (location = 0) in vec3 input_position;
layout (location = 1) in vec3 input_normal;
layout (location = 2) in vec4 input_tangent;
layout (location = 3) in vec2 input_texcoord0;

#include "include/push_constants.h.glsl"

// OUTPUT
layout (location = 0) out vec4 output_position;
layout (location = 1) out vec3 output_normal;
layout (location = 2) out vec3 output_tangent;
layout (location = 3) out vec2 output_tex_coord;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
  // compute world position
  vec4 local_pos = vec4(input_position.xyz, 1.0);
  vec4 world_pos = push_constants.world_matrix * local_pos;
  output_position = world_pos;
  // compute world normal
  vec4 local_normal = vec4(input_normal.xyz, 1.0);
  vec4 world_normal = vec4(normalize((push_constants.world_matrix * local_normal).xyz), 1.0);
  output_normal = normalize(world_normal.xyz);
  // compute world tangent
  vec4 local_tangent = vec4(input_tangent.xyz, 1.0);
  vec4 world_tangent = vec4(normalize((push_constants.world_matrix * local_tangent).xyz), 1.0);
  output_tangent = normalize(world_tangent.xyz);
  // copy texture coordinate
  output_tex_coord = input_texcoord0;
  // project the vertex position into camera
  gl_Position = push_constants.projection_matrix * push_constants.view_matrix * world_pos;
}