#pragma once

// hnll
#include <game/component.hpp>
#include <graphics/renderer.hpp>
#include <utils/utils.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <eigen3/Eigen/Dense>

namespace hnll {

// forward declaration
namespace geometry { class perspective_frustum; }

namespace game {
class viewer_component : public component
{
  public:
    enum class update_view_frustum { ON, OFF };

    viewer_component(hnll::utils::transform &transform, hnll::graphics::renderer &renderer);
    ~viewer_component() override;

    void update_component(float dt) override;

    // getter
    static float     get_near_distance() { return near_distance_; }
    static float     get_fovy()          { return fovy_; }
    const Eigen::Matrix4f& get_projection() const { return projection_matrix_; }
    const Eigen::Matrix4f& get_view()       const { return view_matrix_; }
    Eigen::Matrix4f        get_inverse_perspective_projection() const;
    Eigen::Matrix4f        get_inverse_view_yxz() const;
    const geometry::perspective_frustum& get_perspective_frustum_ref() const { return *perspective_frustum_; }

    // setter
    void set_orthogonal_projection(float left, float right, float top, float bottom, float near, float far);
    void set_perspective_projection(float fovy, float aspect, float near, float far);
    // one of the three ways to initialize view matrix
    // camera position, which direction to point, which direction is up
    void set_view_direction(const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &up = glm::vec3(0.f, -1.f, 0.f));
    void set_view_target(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &up = glm::vec3(0.f, -1.f, 0.f));
    void set_view_yxz();

  private:
    // ref of owner transform
    hnll::utils::transform &transform_;
    Eigen::Matrix4f projection_matrix_ = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f view_matrix_       = Eigen::Matrix4f::Identity();
    hnll::graphics::renderer &renderer_;

    u_ptr<geometry::perspective_frustum> perspective_frustum_;
    update_view_frustum update_view_frustum_ = update_view_frustum::OFF;

    // distance to the screen
    static float near_distance_;
    static float fovy_;
};
}} // namespace hnll::game
