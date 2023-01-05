#pragma once

#include <eigen3/Eigen/Dense>
#include <memory>

using vec2  = Eigen::Vector2f;
using vec3  = Eigen::Vector3f;
using vec4  = Eigen::Vector4f;
using uvec4 = Eigen::Matrix<unsigned, 4, 1>;
using mat4  = Eigen::Matrix4f;
using quat  = Eigen::Quaternionf;
template<class T> using u_ptr = std::unique_ptr<T>;
template<class T> using s_ptr = std::shared_ptr<T>;