// hnll
#include <geometry/collision_detector.hpp>
#include <geometry/bounding_volume.hpp>

// lib
#include <gtest/gtest.h>

using hnll::geometry::collision_detector;
using hnll::geometry::bounding_volume;

collision_detector cdd;

Eigen::Vector3d point5 = {1.f, 0.f, 0.f};
Eigen::Vector3d point6 = {6.f, 0.f, 0.f};
Eigen::Vector3d point3 = {3.f, 0.f, 0.f};
Eigen::Vector3d point4 = {6.f, 3.f, 8.f};
auto sp3 = bounding_volume(point5, 3.f);
auto sp4 = bounding_volume(point6, 4.f);
auto aabb1 = bounding_volume(point3, {3.f, 2.f, 4.f});
auto aabb2 = bounding_volume(point4, {1.f, 0.5f, 4.f});

TEST(bounding_volume_intersection, sphere_sphere)
{
  // intersection
  EXPECT_TRUE(cdd.intersection_bounding_volume(sp3, sp4));
  sp4.set_center_point({1.f, 7.f, 0.f});
  EXPECT_TRUE(cdd.intersection_bounding_volume(sp3, sp4));
  sp4.set_center_point({1.f, 8.f, 0.f});
  EXPECT_FALSE(cdd.intersection_bounding_volume(sp3, sp4));
}

TEST(bounding_volume_intersection, aabb_aabb)
{
  EXPECT_FALSE(cdd.intersection_bounding_volume(aabb1, aabb2));
  aabb2.set_aabb_radius({1.f, 1.f, 4.f});
  EXPECT_TRUE(cdd.intersection_bounding_volume(aabb1, aabb2));
  aabb2.set_aabb_radius({1.f, 1.1f, 4.f});
  EXPECT_TRUE(cdd.intersection_bounding_volume(aabb1, aabb2));
}

TEST(bounding_volume_intersection, aabb_sphere)
{

}