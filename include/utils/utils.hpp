#pragma once

// TODO : put this file on a appropriate position
// TODO : create createOneShotCommandPool();

// std
#include <memory>
#include <iostream>
#include <chrono>

// lib
#include <glm/gtc/matrix_transform.hpp>
#include <eigen3/Eigen/Dense>

namespace hnll {

template<class U> using u_ptr = std::unique_ptr<U>;
template<class S> using s_ptr = std::shared_ptr<S>;

namespace utils {

static std::vector<std::string> loading_directories {
  "/home/honolulu/models/characters",
  "/home/honolulu/models/primitives",
};

std::string get_full_path(const std::string& _filename);
void mkdir_p(const std::string& _dir_name);
// returns cache directory
std::string create_cache_directory();
// returns sub cache directory
std::string create_sub_cache_directory(const std::string& _dir_name);

// 3d transformation
struct transform
{
  glm::vec3 translation{}; // position offset
  glm::vec3 scale{1.f, 1.f, 1.f};
  // y-z-x tait-brian rotation
  glm::vec3 rotation{};

  const Eigen::Vector3d get_translation_eigen() { return Eigen::Vector3d{translation.x, translation.y, translation.z}; };
  const glm::vec3& get_translation_ref() const { return translation; }
  const glm::vec3& get_scale_ref() const       { return scale; }
  const glm::vec3& get_rotation_ref() const    { return rotation; }

  // Matrix corresponds to Translate * Ry * Rz * Rx * Scale
  // Rotations correspond to Tait-bryan angles of Y(1), Z(2), X(3)
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  Eigen::Matrix4d mat4() const;
  Eigen::Matrix4d rotate_mat4() const;
  Eigen::Matrix3d rotate_mat3() const;
  // normal = R * S(-1)
  Eigen::Matrix4d normal_matrix() const;
};

struct scope_timer {
  // start timer by ctor
  scope_timer(const std::string& _entry = "") {
    entry = _entry;
    start = std::chrono::system_clock::now();
  }

  // stop and output elapsed time by dtor
  ~scope_timer() {
    auto end = std::chrono::system_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "\x1b[35m" << "SCOPE TIMER : " << entry << std::endl;
    std::cout << "\t elapsed time : " << elapsed << " micro s" << "\x1b[0m" << std::endl;
  }

  std::string entry;
  std::chrono::system_clock::time_point start;
};

static inline glm::vec3 sclXvec(const float scalar, const glm::vec3& vec)
{ return {vec.x * scalar, vec.y * scalar, vec.z * scalar}; }

template <class V>
concept glm_vec = requires(V& vec){ glm::normalize(vec); };
// glm rotation direction (true : regular direction, false : irregular direction)
inline bool is_same_handed_system(const glm_vec auto& a1, const glm_vec auto& a2, const glm_vec auto& b1, const glm_vec auto& b2)
{
  if (glm::cross(a1, a2).z * glm::cross(b1, b2).z < 0) return false;
  return true;
}

} // namespace utils
} // namespace hnll