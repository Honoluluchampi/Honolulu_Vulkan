// hnll
#include <graphics/device.hpp>
#include <graphics/buffer.hpp>

// sub
#include <extensions_vk.hpp>

// lib
#include <eigen3/Eigen/Dense>

namespace hnll {

using vec3 = Eigen::Vector3f;
template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

struct acceleration_structure
{
  VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
  VkDeviceMemory             memory = VK_NULL_HANDLE;
  VkBuffer                   buffer = VK_NULL_HANDLE;
  VkDeviceAddress            device_address = 0;
};

struct ray_tracing_scratch_buffer
{
  VkBuffer        handle = VK_NULL_HANDLE;
  VkDeviceMemory  memory = VK_NULL_HANDLE;
  VkDeviceAddress device_address = 0;
};

VkDeviceAddress get_device_address(VkDevice device, VkBuffer buffer)
{
  VkBufferDeviceAddressInfo buffer_device_info {
    VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
    nullptr
  };
  buffer_device_info.buffer = buffer;
  return vkGetBufferDeviceAddress(device, &buffer_device_info);
}

class hello_triangle {
  public:
    hello_triangle()
    {
      window_ = std::make_unique<graphics::window>(1920, 1080, "hello ray tracing triangle");
      device_ = std::make_unique<graphics::device>(*window_, graphics::rendering_type::RAY_TRACING);

      // load all available extensions (of course including ray tracing extensions)
      load_VK_EXTENSIONS(device_->get_instance(), vkGetInstanceProcAddr, device_->get_device(), vkGetDeviceProcAddr);

      create_triangle_as();
    }

    ~hello_triangle()
    {
      auto device = device_->get_device();
      vkDestroyAccelerationStructureKHR(device, blas_->handle, nullptr);
      vkFreeMemory(device, blas_->memory, nullptr);
      vkDestroyBuffer(device, blas_->buffer, nullptr);
    }

  private:
    void create_triangle_as()
    {
      create_vertex_buffer();
      create_triangle_blas();
    }

    void create_vertex_buffer()
    {
      uint32_t vertex_count = triangle_vertices_.size();
      uint32_t vertex_size = sizeof(triangle_vertices_[0]);
      VkDeviceSize buffer_size = vertex_size * vertex_count;

      // create staging buffer
      graphics::buffer staging_buffer{
          *device_,
          vertex_size,
          vertex_count,
          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      };
      staging_buffer.map();
      staging_buffer.write_to_buffer((void *) triangle_vertices_.data());

      // setup vertex buffer create info
      VkBufferUsageFlags usage =
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
          VK_BUFFER_USAGE_TRANSFER_DST_BIT |
          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
          VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

      // create vertex buffer
      vertex_buffer_ = std::make_unique<graphics::buffer>(
          *device_,
          vertex_size,
          vertex_count,
          usage,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      );
      // write data to the buffer
      device_->copy_buffer(staging_buffer.get_buffer(), vertex_buffer_->get_buffer(), buffer_size);
    }

    void create_triangle_blas()
    {
      // blas build setup

      // get vertex buffer device address
      VkDeviceOrHostAddressConstKHR vertex_buffer_device_address {};
      vertex_buffer_device_address.deviceAddress =
          get_device_address(device_->get_device(), vertex_buffer_->get_buffer());

      // geometry
      VkAccelerationStructureGeometryKHR as_geometry {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR
      };
      as_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
      as_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
      as_geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
      as_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
      as_geometry.geometry.triangles.vertexData = vertex_buffer_device_address;
      as_geometry.geometry.triangles.maxVertex = triangle_vertices_.size();
      as_geometry.geometry.triangles.vertexStride = sizeof(vec3);
      as_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;

      // build geometry info
      VkAccelerationStructureBuildGeometryInfoKHR as_build_geometry_info {};
      as_build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
      as_build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
      // prefer performance rather than as build
      as_build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
      as_build_geometry_info.geometryCount = 1; // only one triangle
      as_build_geometry_info.pGeometries = &as_geometry;

      // get as size
      uint32_t num_triangles = 1;
      VkAccelerationStructureBuildSizesInfoKHR as_build_sizes_info {
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
      };
      vkGetAccelerationStructureBuildSizesKHR(
        device_->get_device(),
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &as_build_geometry_info,
        &num_triangles,
        &as_build_sizes_info
      );

      // build blas (get handle of VkAccelerationStructureKHR)
      blas_ = create_acceleration_structure_buffer(as_build_sizes_info);

      // create blas
      VkAccelerationStructureCreateInfoKHR as_create_info {};
      as_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
      as_create_info.buffer = blas_->buffer;
      as_create_info.size = as_build_sizes_info.accelerationStructureSize;
      as_create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
      vkCreateAccelerationStructureKHR(device_->get_device(), &as_create_info, nullptr, &blas_->handle);

      // get the device address of blas
      VkAccelerationStructureDeviceAddressInfoKHR as_device_address_info {};
      as_device_address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
      as_device_address_info.accelerationStructure = blas_->handle;
      blas_->device_address = vkGetAccelerationStructureDeviceAddressKHR(device_->get_device(), &as_device_address_info);

    }

    u_ptr<acceleration_structure> create_acceleration_structure_buffer
        (VkAccelerationStructureBuildSizesInfoKHR build_size_info)
    {
      auto as = std::make_unique<acceleration_structure>();
      auto usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

      device_->create_buffer(
        build_size_info.accelerationStructureSize,
        usage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        as->buffer,
        as->memory
      );

      return as;
    }

    u_ptr<ray_tracing_scratch_buffer> create_scratch_buffer(VkDeviceSize size)
    {
      auto scratch_buffer = std::make_unique<ray_tracing_scratch_buffer>();
      auto usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

      device_->create_buffer(
        size,
        usage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        scratch_buffer->handle,
        scratch_buffer->memory
      );

      scratch_buffer->device_address = get_device_address(device_->get_device(), scratch_buffer->handle);
      return scratch_buffer;
    }

    // variables
    u_ptr<graphics::window> window_;
    u_ptr<graphics::device> device_;
    u_ptr<graphics::buffer> vertex_buffer_;
    std::vector<vec3> triangle_vertices_ = {
        {-0.5f, -0.5f, 0.0f},
        {+0.5f, -0.5f, 0.0f},
        {0.0f,  0.75f, 0.0f}
    };

    // acceleration structure
    u_ptr<acceleration_structure> blas_;
    u_ptr<acceleration_structure> tlas_;
};
}

int main() {
  hnll::hello_triangle app;
}

// empty
#include <geometry/mesh_model.hpp>
void hnll::geometry::mesh_model::align_vertex_id() {}
