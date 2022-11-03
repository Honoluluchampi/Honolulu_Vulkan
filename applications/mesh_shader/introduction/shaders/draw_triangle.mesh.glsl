#version 450
#extension GL_NV_mesh_shader : require

// the second variable of vkCmdDrawMeshTasksNV()
layout(local_size_x = 1) in;

const uint MAX_VERTEX_COUNT = 64;
const uint MAX_PRIMITIVE_INDICES_COUNT = 378;
const uint MAX_MESHLET_COUNT = 10;

// identifier "triangles" indicates this shader outputs trianlges (other candidates : point, line)
// gl_MeshVerticesNV and glPrimitiveIndicesNV is resized according to these values
layout(triangles, max_vertices = MAX_VERTEX_COUNT, max_primitives = MAX_PRIMITIVE_INDICES_COUNT) out;

// pass to fragment shader
layout (location = 0) out PerVertexData {
  vec4 color;
} v_out[];

// bindings
// ------------------------------------------------------------------------
// scene info
//layout(set = 0, binding = 0) uniform GlobalUbo
//{
//  mat4 projection;
//  mat4 view;
//  mat4 inv_view;
//  vec4 ambient_light_color;
//  PointLight point_lights[20];
//  int num_lights;
//} ubo;

// ------------------------------------------------------------------------
// vertex buffer
struct vertex {
  vec3 position;
  vec3 normal;
  vec3 color;
};

layout(set = 1, binding = 0) uniform vertex_bufer {
  vertex raw_vertices[MAX_MESHLET_COUNT * MAX_VERTEX_COUNT];
};

// ------------------------------------------------------------------------
// meshlet buffer

struct meshlet {
  uint vertex_indices[MAX_VERTEX_COUNT];
  uint primitive_indices[MAX_PRIMITIVE_INDICES_COUNT];
  uint vertex_count; // < MAX_VERTEX_COUNT
  uint index_count; // < MAX_PRIMITIVE_INDICES_COUNT
};

layout(set = 1, binding = 2) uniform mesh_buffer {
  meshlet meshlets[];
};

// ------------------------------------------------------------------------

void main() {

  // following three gl_~NV variables are built in variables for mesh shading

  uint mesh_index = gl_WorkGroupID.x;
  uint thread_id = gl_LocalInvocationID.x;

  for (uint i = 0; i < meshlets[mesh_index].vertex_count; i++) {
    // i indicates gl_~'s index
    // vertex_index indicates the vertex_buffer's index
    uint vertex_index = meshlets[mesh_index].vertex_indices[i];

    gl_MeshVerticesNV[i].gl_Position = vec4(raw_vertices[vertex_index].position, 1.f);
    v_out[i].color = vec4(raw_vertices[vertex_index].color, 1);
  }

  uint index_count = meshlets[mesh_index].index_count;
  gl_PrimitiveCountNV = uint(index_count) / 3;

  for (uint i = 0; i < index_count; i++) {
    gl_PrimitiveIndicesNV[i] = uint(meshlets[mesh_index].primitive_indices[i]);
  }
}