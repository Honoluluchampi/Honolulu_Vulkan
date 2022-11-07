#version 460
#extension GL_NV_mesh_shader : require
#extension GL_EXT_shader_explicit_arithmetic_types : require

// the second variable of vkCmdDrawMeshTasksNV()
layout(local_size_x = 2) in;

const uint MAX_VERTEX_COUNT = 64;
const uint MAX_PRIMITIVE_INDICES_COUNT = 378;
const uint MAX_MESHLET_COUNT = 10;
const uint MESHLET_PER_TASK = 32;

// identifier "triangles" indicates this shader outputs trianlges (other candidates : point, line)
// gl_MeshVerticesNV and glPrimitiveIndicesNV is resized according to these values
layout(triangles, max_vertices = MAX_VERTEX_COUNT, max_primitives = MAX_PRIMITIVE_INDICES_COUNT / 3) out;

// inputs from task shader
//taskNV in Task {
taskNV in task {
  uint    baseID;
  uint8_t deltaIDs[MESHLET_PER_TASK];
} IN;
// gl_WorkGroupID.x runs from [0 .. parentTask.gl_TaskCountNV - 1]
uint meshletID = IN.baseID + IN.deltaIDs[gl_WorkGroupID.x];

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
// meshlet buffer
// one storage buffer binding can have only one flexible length array

struct vertex {
  vec3 position;
  vec3 normal;
  vec3 color;
};

layout(set = 0, binding = 0) buffer _vertex_buffer {
  vertex raw_vertices[];
};

struct meshlet {
  uint vertex_indices   [MAX_VERTEX_COUNT];
  uint primitive_indices[MAX_PRIMITIVE_INDICES_COUNT];
  uint vertex_count; // < MAX_VERTEX_COUNT
  uint index_count; // < MAX_PRIMITIVE_INDICES_COUNT
};

layout(set = 1, binding = 0) buffer _mesh_buffer {
  meshlet meshlets[];
};

// ------------------------------------------------------------------------

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

  // following three gl_~NV variables are built in variables for mesh shading
  uint meshlet_index = gl_WorkGroupID.x;
  meshlet current_meshlet = meshlets[meshlet_index];

  //------- vertex processing ---------------------------------------------
  uint vertex_count = current_meshlet.vertex_count;

  for (uint i = 0; i < vertex_count; i++) {
    // i indicates gl_~'s index
    // vertex_index indicates the vertex_buffer's index
    uint vertex_index = current_meshlet.vertex_indices[i];

    gl_MeshVerticesNV[i].gl_Position = vec4(raw_vertices[vertex_index].position, 1.f);
    v_out[i].color = vec4(meshlet_colors[meshlet_index %COLOR_COUNT], 1.f);
  }

  //------- index processing ----------------------------------------------
  uint index_count = current_meshlet.index_count;
  gl_PrimitiveCountNV = uint(index_count) / 3;

  for (uint i = 0; i < index_count; i++) {
    gl_PrimitiveIndicesNV[i] = current_meshlet.primitive_indices[i];
  }
}