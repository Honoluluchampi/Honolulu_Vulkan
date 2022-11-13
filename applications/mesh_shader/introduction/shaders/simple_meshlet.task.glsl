#version 460

#extension GL_NV_mesh_shader : require
#extension GL_EXT_shader_8bit_storage : require

const uint MAX_VERTEX_COUNT = 64;
const uint MAX_PRIMITIVE_INDICES_COUNT = 378;
const uint WORKGROUP_SIZE   = 32;
const uint MESHLET_PER_TASK = 32;
const uint TASK_MESHLET_ITERATION =
  (MESHLET_PER_TASK + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;

uint base_id = gl_WorkGroupID.x * MESHLET_PER_TASK;
uint lane_id = gl_LocalInvocationID.x;

// -------------------------------------------------------

layout(local_size_x = WORKGROUP_SIZE) in;

taskNV out task {
    uint base_id;
    uint8_t sub_ids[MESHLET_PER_TASK];
} OUT;

// bindings
// -------------------------------------------------------
// scene info

struct PointLight {
  vec4 position;
  vec4 color;
};

layout(set = 0, binding = 0) uniform _global_ubo {
  mat4 projection;
  mat4 view;
  mat4 inv_view;
  vec4 ambient_light_color;
  PointLight point_lights[20];
  int num_lights;
} ubo;

// ------------------------------------------------------
// meshlet info

struct meshlet {
  uint vertex_indices   [MAX_VERTEX_COUNT];
  uint primitive_indices[MAX_PRIMITIVE_INDICES_COUNT];
  uint vertex_count; // < MAX_VERTEX_COUNT
  uint index_count; // < MAX_PRIMITIVE_INDICES_COUNT
  // for frustum culling
  vec3 center;
  float radius;
};

layout(set = 2, binding = 0) buffer _mesh_buffer {
  meshlet meshlets[];
};

// ------------------------------------------------------
// frustum info

// top, bottom, right, left have same position (camera position)
struct frustum_info {
  vec3 camera_position;
  vec3 near_position;
  vec3 far_position;
  vec3 top_n;
  vec3 bottom_n;
  vec3 right_n;
  vec3 left_n;
  vec3 near_n;
  vec3 far_n;
};

layout(set = 3, binding = 0) uniform _frustum_info {
  frustum_info frustum;
};

// ------------------------------------------------------

layout(push_constant) uniform Push {
  mat4 model_matrix;
  mat4 normal_matrix;
} push;

// ------------------------------------------------------

// signed distance
double distance_point_to_plane(vec3 point, vec3 plane_position, vec3 plane_normal) {
  return dot(plane_normal, point - plane_position);
}

// returns true if an object is (partly) included by the frustum
bool sphere_frustum_intersection(vec3 world_center, float radius) {
  bool top    = distance_point_to_plane(world_center, frustum.camera_position, frustum.top_n)    > -radius;
  bool bottom = distance_point_to_plane(world_center, frustum.camera_position, frustum.bottom_n) > -radius;
  bool right  = distance_point_to_plane(world_center, frustum.camera_position, frustum.right_n)  > -radius;
  bool left   = distance_point_to_plane(world_center, frustum.camera_position, frustum.left_n)   > -radius;
  bool near   = distance_point_to_plane(world_center, frustum.near_position,   frustum.near_n)   > -radius;
  bool far    = distance_point_to_plane(world_center, frustum.far_position,    frustum.far_n)    > -radius;
  return top && bottom && right && left && near && far; 
}

void main() {
    uint out_meshlet_count = 0;
    uint meshlet_count = meshlets.length();

    for (uint i = 0; i < MESHLET_PER_TASK; i++) {
      uint meshlet_local = lane_id + i;
      uint current_meshlet_index = base_id + meshlet_local;
      meshlet current_meshlet = meshlets[current_meshlet_index];

      vec4 world_center = push.model_matrix * vec4(current_meshlet.center, 1.0);

      if (sphere_frustum_intersection(world_center.xyz, current_meshlet.radius)) {
        OUT.sub_ids[out_meshlet_count] = uint8_t(meshlet_local);
        out_meshlet_count += 1;
      }
    }

    if (lane_id == 0) {
        gl_TaskCountNV = out_meshlet_count;
        OUT.base_id    = base_id;
    }
}