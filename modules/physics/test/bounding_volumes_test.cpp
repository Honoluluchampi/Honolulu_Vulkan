// hnll
#include <physics/bounding_volumes/bounding_sphere.hpp>

// lib
#include <gtest/gtest.h>

using namespace hnll::physics;

TEST(bounding_volumes, sphere){
  Eigen::Vector3d point1 = {1.f, 0.f, 0.f};
  Eigen::Vector3d point2 = {6.f, 0.f, 0.f};
  bounding_sphere sp1 = {point1, 3.f};
  bounding_sphere sp2 = {point2, 4.f};
  // ctor
  EXPECT_EQ(sp1.get_center_point(), point1);
  EXPECT_EQ(sp1.get_radius(), 3.f);
  // intersection
  EXPECT_TRUE(sp1.intersect_with(sp2));
  sp2.set_center_point({1.f, 7.f, 0.f});
  EXPECT_TRUE(sp1.intersect_with(sp2));
  sp2.set_center_point({1.f, 8.f, 0.f});
  EXPECT_FALSE(sp1.intersect_with(sp2));
}