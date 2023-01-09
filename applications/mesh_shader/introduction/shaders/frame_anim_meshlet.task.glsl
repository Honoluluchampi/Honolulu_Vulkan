#version 460

#extension GL_NV_mesh_shader          : require
#extension GL_EXT_shader_8bit_storage : require

const uint MAX_VERTEX_COUNT            = 64;
const uint MAX_PRIMITIVE_INDICES_COUNT = 378;
const uint MESHLET_PER_TASK = 32;

uint base_id = gl_WorkGroupID.x * MESHLET_PER_TASK;
uint lane_id = gl_LocalInvocationID.x;

// -------------------------------------------------------

layout(local_size_x = MESHLET_PER_TASK) in;

taskNV out task {
    uint base_id;
    uint8_t sub_ids[MESHLET_PER_TASK];
} OUT;

// bindings
// ------------------------------------------------------------------------
// scene info

struct PointLight
{
  vec4 position;
  vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo
{
  mat4 projection;
  mat4 view;
  mat4 inv_view;
  vec4 ambient_light_color;
  PointLight point_lights[20];
  int num_lights;
} ubo;

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

layout(set = 1, binding = 0) uniform _frustum_info {
  frustum_info frustum;
};

// ------------------------------------------------------------------------
// meshlet buffer
// one storage buffer binding can have only one flexible length array

struct meshlet {
  uint vertex_indices   [MAX_VERTEX_COUNT];
  uint primitive_indices[MAX_PRIMITIVE_INDICES_COUNT];
  uint vertex_count; // < MAX_VERTEX_COUNT
  uint index_count; // < MAX_PRIMITIVE_INDICES_COUNT
};

struct sphere {
  vec4 center_and_radius; // .xyz : center position, .w : radius
};

struct common_attribute {
  vec2 uv0;
  vec2 uv1;
  vec4 color;
};

struct dynamic_attribute {
  vec3 position;
  vec3 normal;
};

layout(set = 2, binding = 0) buffer CommonAttribs {
  common_attribute common_attributes[];
};

layout(set = 3, binding = 0) buffer MeshBuffer {
  meshlet meshlets[];
};

layout(set = 4, binding = 0) buffer DynamicAttribs {
  dynamic_attribute dynamic_attributes[];
};

layout(set = 5, binding = 0) buffer BoundingSphere {
  sphere bounding_sphere[];
};

// ------------------------------------------------------------------------

layout(push_constant) uniform Push {
  mat4 model_matrix;
  mat4 normal_matrix;
} push;

#define COLOR_COUNT 10
vec3 meshlet_colors[COLOR_COUNT] = {
  vec3(1,0,0),
  vec3(0,1,0),
  vec3(0,0,1),
  vec3(1,1,0),
  vec3(1,0,1),
  vec3(0,1,1),
  vec3(1,0.5,0),
  vec3(0.5,1,0),
  vec3(0,0.5,1),
  vec3(1,1,1)
};

void main() {
    uint out_meshlet_count = 0;

    for (uint i = 0; i < MESHLET_PER_TASK; i++) {
      uint meshlet_local = lane_id + i;
      uint current_meshlet_index = base_id + meshlet_local;
      meshlet current_meshlet = meshlets[current_meshlet_index];

 //     vec4 world_center = push.model_matrix * vec4(current_meshlet.center, 1.0);

      //if (sphere_frustum_intersection(world_center.xyz, current_meshlet.radius)) {
        OUT.sub_ids[out_meshlet_count] = uint8_t(meshlet_local);
        out_meshlet_count += 1;
      //}
    }

    if (lane_id == 0) {
        gl_TaskCountNV = out_meshlet_count;
        OUT.base_id    = base_id;
    }
}