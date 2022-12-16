#version 450

layout(location = 0) in vec3  in_position;
layout(location = 1) in vec3  in_normal;
layout(location = 2) in vec2  in_tex_coord;
layout(location = 3) in uvec4 in_joint_indices;
layout(location = 4) in vec4  in_joint_weights;

layout(location = 0) out vec3 out_world_position;
layout(location = 0) out vec3 out_world_normal;

// should be compatible with a description layout
struct PointLight
{
  vec4 position;
  vec4 color;
};

// global desc set
layout(set = 0, binding = 0) uniform GlobalUbo
{
  mat4 projection;
  mat4 view;
  mat4 inv_view;
  vec4 ambientLightColor;
  PointLight pointLights[20];
  int numLights;
} ubo;

#define MAX_NUM_JOINTS 128

layout (set = 1, binding = 0) uniform UboNode {
  mat4 matrix;
  mat4 joint_matrices[MAX_NUM_JOINTS];
  float joint_count;
} node;

// compatible with a renderer system
layout(push_constant) uniform Push {
  mat4 model_matrix;
  mat4 normal_matrix;
} push;

int main() {
  vec4 local_position;
  // if mesh is skinned 
  if (node.joint_count > 0.0) {
    mat4 skin_mat = 
      in_joint_weights.x * node.joint_matrices[int(in_joint_indices.x)] + 
      in_joint_weights.y * node.joint_matrices[int(in_joint_indices.y)] + 
      in_joint_weights.z * node.joint_matrices[int(in_joint_indices.z)] + 
      in_joint_weights.w * node.joint_matrices[int(in_joint_indices.w)];

    // mat4 transform_matrix = push.model_matrix * node.matrix * skin_mat;
    local_position = push.model_matrix * node.matrix * skin_mat * vec4(in_position, 1.0);
    out_world_normal = normalize(transpose(inverse(mat3(push.model_matrix * node.matrix * skin_mat))) * in_normal);
  } else {
    local_position = push.model_matrix * node.matrix * vec4(in_position, 1.0);
    out_world_normal = normalize(transpose(inverse(mat3(push.model_matrix * node.matrix))) * in_normal);
  }

  out_world_position = local_position.xyz / local_position.w;
  gl_Position = ubo.projection * ubo.view * vec4(out_world_position, 1.0);
}