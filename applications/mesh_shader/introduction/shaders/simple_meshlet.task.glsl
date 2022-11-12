#version 460

#extension GL_NV_mesh_shader : require
#extension GL_EXT_shader_8bit_storage : require

const uint MAX_VERTEX_COUNT = 64;
const uint MAX_PRIMITIVE_INDICES_COUNT = 378;
const uint WORKGROUP_SIZE   = 32;
const uint MESHLET_PER_TASK = 32;
const uint TASK_MESHLET_ITERATION =
  (MESHLET_PER_TASK + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;

layout(local_size_x = WORKGROUP_SIZE) in;

uint base_id = gl_WorkGroupID.x * MESHLET_PER_TASK;
uint lane_id = gl_LocalInvocationID.x;

//taskNV out task {
taskNV out task {
    uint base_id;
    uint8_t sub_ids[MESHLET_PER_TASK];
} OUT;

struct meshlet {
  uint vertex_indices   [MAX_VERTEX_COUNT];
  uint primitive_indices[MAX_PRIMITIVE_INDICES_COUNT];
  uint vertex_count; // < MAX_VERTEX_COUNT
  uint index_count; // < MAX_PRIMITIVE_INDICES_COUNT
};

layout(set = 2, binding = 0) buffer _mesh_buffer {
  meshlet meshlets[];
};

void main() {
    uint out_meshlet_count = 0;
    uint meshlet_count = meshlets.length();
    bool render = gl_GlobalInvocationID.x < meshlet_count && (gl_GlobalInvocationID.x % 2 == 0);

    for (uint i = 0; i < TASK_MESHLET_ITERATION; i++) {
        uint meshlet_local = lane_id + i * WORKGROUP_SIZE;
        uint meshlet_global = base_id + meshlet_local;

        OUT.sub_ids[out_meshlet_count] = uint8_t(meshlet_local);
        out_meshlet_count += 1;
    }

    if (lane_id == 0) {
        gl_TaskCountNV = out_meshlet_count;
        OUT.base_id    = base_id;
    }
}