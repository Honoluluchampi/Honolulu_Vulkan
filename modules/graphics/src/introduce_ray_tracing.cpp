// hnll
#include <graphics/device.hpp>
#include <graphics/buffer.hpp>

// lib
#include <eigen3/Eigen/Dense>

namespace hnll {

using vec3 = Eigen::Vector3f;
template<typename T> using u_ptr = std::unique_ptr<T>;
template<typename T> using s_ptr = std::shared_ptr<T>;

uint64_t get_device_address(VkDevice device, VkBuffer buffer)
{
  VkBufferDeviceAddressInfoKHR buffer_device_info {
    VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
    nullptr
  };
  buffer_device_info.buffer = buffer;
  return vkGetBufferDeviceAddressKHR(device, &buffer_device_info);
}

class hello_triangle
{
  public:
    hello_triangle()
    {
      window_ = std::make_unique<graphics::window>(1920, 1080, "hello ray tracing triangle");
      device_ = std::make_unique<graphics::device>(*window_);
      device_->set_rendering_type(graphics::rendering_type::RAY_TRACING);

      create_triangle_blas();
    }

  private:
    void create_triangle_blas()
    {
      create_vertex_buffer();
    }

    void create_vertex_buffer()
    {
      std::vector<vec3> triangle_vertices = {
          {-0.5f, -0.5f, 0.0f},
          {+0.5f, -0.5f, 0.0f},
          {0.0f, 0.75f, 0.0f}
      };

      uint32_t vertex_count = 3;
      uint32_t vertex_size = sizeof(triangle_vertices[0]);
      VkDeviceSize buffer_size = vertex_size * vertex_count;

      // create staging buffer
      graphics::buffer staging_buffer {
          *device_,
          vertex_size,
          vertex_count,
          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
      };
      staging_buffer.map();
      staging_buffer.write_to_buffer((void *)triangle_vertices.data());

      // setup vertex buffer create info
      VkBufferUsageFlags  usage =
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

    u_ptr<graphics::window> window_;
    u_ptr<graphics::device> device_;
    u_ptr<graphics::buffer> vertex_buffer_;
};
}

int main() {
  hnll::hello_triangle app;
}

// empty
#include <geometry/mesh_model.hpp>
void hnll::geometry::mesh_model::align_vertex_id() {}
