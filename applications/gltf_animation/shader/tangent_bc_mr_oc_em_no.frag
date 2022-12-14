#version 450

#extension GL_GOOGLE_include_directive     : enable
#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

#include "include/io_with_tangent.h.glsl"
#include "include/constants.h.glsl"
#include "include/push_constants.h.glsl"
#include "include/lighting.h.glsl"

// binding 0 is used by push_constants.h.glsl
layout (binding = 1) uniform sampler2D base_color;
layout (binding = 2) uniform sampler2D metallic_roughness;
layout (binding = 3) uniform sampler2D normal_map;
layout (binding = 4) uniform sampler2D occlusion;
layout (binding = 5) uniform sampler2D emissive;

void main() {
  vec3 normal   = normalize(input_normal.xyz);
  vec3 tangent  = normalize(input_tangent.xyz);
  vec3 binormal = cross(tangent, normal);
  // transformation matrix from world to tangent space
  mat3 ts = transpose(mat3(tangent, binormal, normal));
  
  vec3 pos = input_position.xyz;
  
  vec3 N = normalize(texture(normal_map, input_texcoord).rgb * vec3(materials.normal_scale, materials.normal_scale, 1) * 2.0 - 1.0);
  vec3 V = ts * normalize(push_constants.camera_pos.xyz-pos);
  vec3 L = ts * normalize(push_constants.light_pos.xyz-pos);
  
  vec4 mr = texture(metallic_roughness, input_texcoord);
  float roughness    = mr.g * materials.roughness;
  float metallicness = mr.b * materials.metalness;

  vec4  diffuse_color  = texture(base_color, input_texcoord) * materials.base_color;
  float ambient        = 0.05 * mix(1 - materials.occlusion_strength, 1, texture(occlusion, input_texcoord).r);
  vec3  emissive       = materials.emissive.rgb * texture(emissive, input_texcoord).rgb;
  vec3  linear         = light(L, V, N, diffuse_color.rgb, emissive, roughness, metallicness, ambient);

  output_color = vec4(gamma(linear), diffuse_color.a); 
}