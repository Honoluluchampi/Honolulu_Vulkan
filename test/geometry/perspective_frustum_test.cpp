// hnll
#include <geometry/perspective_frustum.hpp>
#include <utils/utils.hpp>

// lib
#include <gtest/gtest.h>

using namespace hnll::geometry;
using vec3 = Eigen::Vector3d;

namespace hnll::geometry {
bool operator== (const plane& rhs, const plane& lhs) {
  double eps = 0.0001;
  bool point_eq  = (std::abs(rhs.point.x() - lhs.point.x()) < eps) && (std::abs(rhs.point.y() - lhs.point.y()) < eps) && (std::abs(rhs.point.z() - lhs.point.z()) < eps);
  if (!point_eq) return false;
  bool normal_eq = (std::abs(rhs.normal.x() - lhs.normal.x()) < eps) && (std::abs(rhs.normal.y() - lhs.normal.y()) < eps) && (std::abs(rhs.normal.z() - lhs.normal.z()) < eps);
  return normal_eq;
}
}

TEST(perspective_frustum_test, ctor) {
  perspective_frustum frustum = {M_PI / 1.5f, M_PI / 1.5f, 3, 9};
  auto near   = frustum.get_near_ref();
  auto far    = frustum.get_far_ref();
  auto right  = frustum.get_right_ref();
  auto left   = frustum.get_left_ref();
  auto top    = frustum.get_top_ref();
  auto bottom = frustum.get_bottom_ref();
  plane my_near   = {vec3(0.f, 0.f, 3.f), vec3{0.f, 0.f, 1.f}};
  plane my_far    = {vec3(0.f, 0.f, 9.f), vec3{0.f, 0.f, -1.f}};
  plane my_right  = {vec3(0.f, 0.f, 0.f), vec3{-std::cos(M_PI / 3.f), 0.f, std::sin(M_PI / 3.f)}};
  plane my_left   = {vec3(0.f, 0.f, 0.f), vec3{std::cos(M_PI / 3.f), 0.f,  std::sin(M_PI / 3.f)}};
  plane my_top    = {vec3(0.f, 0.f, 0.f), vec3{0.f, std::cos(M_PI / 3.f),  std::sin(M_PI / 3.f)}};
  plane my_bottom = {vec3(0.f, 0.f, 0.f), vec3{0.f, -std::cos(M_PI / 3.f), std::sin(M_PI / 3.f)}};
  EXPECT_EQ(near,   my_near);
  EXPECT_EQ(far,    my_far);
  EXPECT_EQ(right,  my_right);
  EXPECT_EQ(left,   my_left);
  EXPECT_EQ(top,    my_top);
  EXPECT_EQ(bottom, my_bottom);
}

TEST(perspective_frustum_test, transform) {
  perspective_frustum frustum = {M_PI / 1.5f, M_PI / 1.5f, 3, 9};
  hnll::utils::transform tf;
  tf.translation = {3, 4, 8};
  tf.rotation    = {M_PI / 3.f, 0.f, 0.f};
  auto rotate_mat = tf.rotate_mat3();

  // update its transform
  frustum.update_planes(tf);

  auto near   = frustum.get_near_ref();
  auto far    = frustum.get_far_ref();
  auto right  = frustum.get_right_ref();
  auto left   = frustum.get_left_ref();
  auto top    = frustum.get_top_ref();
  auto bottom = frustum.get_bottom_ref();
  plane my_near   = {vec3(3.f, 4 -3.f * std::sin(M_PI / 3.f), 8 + 1.5f), vec3{0.f, -std::sin(M_PI / 3.f), std::cos(M_PI / 3.f)}};
  plane my_far    = {vec3(3.f, 4 -9.f * std::sin(M_PI / 3.f), 8 + 4.5f), vec3{0.f, std::sin(M_PI / 3.f), -0.5f}};
//  plane my_right  = {vec3(3, 4, 8), vec3{-std::cos(M_PI / 3.f), 0.f, std::sin(M_PI / 3.f)}};
//  plane my_left   = {vec3(3, 4, 8), vec3{std::cos(M_PI / 3.f), 0.f,  std::sin(M_PI / 3.f)}};
//  plane my_top    = {vec3(3, 4, 8), vec3{0.f, std::cos(M_PI / 3.f),  std::sin(M_PI / 3.f)}};
//  plane my_bottom = {vec3(3, 4, 8), vec3{0.f, -std::cos(M_PI / 3.f), std::sin(M_PI / 3.f)}};

  EXPECT_EQ(near,   my_near);
  EXPECT_EQ(far,    my_far);
//  EXPECT_EQ(right,  my_right);
//  EXPECT_EQ(left,   my_left);
//  EXPECT_EQ(top,    my_top);
//  EXPECT_EQ(bottom, my_bottom);
}