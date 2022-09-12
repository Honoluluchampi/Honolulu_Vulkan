// hnll
#include <physics/bounding_volume.hpp>
#include <physics/collision_detector.hpp>
// lib
#include <gtest/gtest.h>

using namespace hnll::physics;

Eigen::Vector3d point1 = {1.f, 0.f, 0.f};
Eigen::Vector3d point2 = {6.f, 0.f, 0.f};
auto sp1 = bounding_volume(point1, 3.f);
auto sp2 = bounding_volume(point2, 4.f);

collision_detector cd;

TEST(bounding_sphere, ctor){
  // ctor
  EXPECT_EQ(sp1.get_center_point(), point1);
  EXPECT_EQ(sp1.get_sphere_radius(), 3.f);
}

TEST(bounding_sphere, intersection){
  // intersection
  EXPECT_TRUE(cd.intersection_test(sp1, sp2));
  sp2.set_center_point({1.f, 7.f, 0.f});
  EXPECT_TRUE(cd.intersection_test(sp1, sp2));
  sp2.set_center_point({1.f, 8.f, 0.f});
  EXPECT_FALSE(cd.intersection_test(sp1, sp2));
}

std::vector<Eigen::Vector3d> sample_vertices{
  {1.f, 1.f, 0.f},
  {-1.f, 1.f, 0.f},
  {0.f, 1.5f, 0.f},
  {0.f, 0.5f, 0.f},
  {0.f, 1.f, 0.5f},
  {0.f, 1.f, -0.5f}
};

TEST(bounding_sphere, most_separated_points_on_aabb){
  auto separated_points = most_separated_points_on_aabb(sample_vertices);
  EXPECT_TRUE(separated_points.first == 1);
  EXPECT_TRUE(separated_points.second == 0);
}

TEST(bounding_sphere, sphere_from_distant_points){
  auto sphere = sphere_from_distant_points(sample_vertices);
  EXPECT_DOUBLE_EQ(sphere->get_sphere_radius(), 1.f);
  EXPECT_TRUE(sphere->get_center_point().x() == 0.f);
  EXPECT_TRUE(sphere->get_center_point().y() == 1.f);
}

TEST(bounding_sphere, extend_sphere_to_point){
  Eigen::Vector3d point{2.f, 1.f, 0.f};
  auto sphere = sphere_from_distant_points(sample_vertices);
  extend_sphere_to_point(*sphere, point);
  EXPECT_EQ(sphere->get_center_point().x(), 0.5f);
  EXPECT_EQ(sphere->get_center_point().y(), 1.f);
}

std::vector<Eigen::Vector3d> cube {
  {1.f, 1.f, 1.f},
  {1.f, 1.f, -1.f},
  {1.f, -1.f, 1.f},
  {1.f, -1.f, -1.f},
  {-1.f, 1.f, 1.f},
  {-1.f, 1.f, -1.f},
  {-1.f, -1.f, 1.f},
  {-1.f, -1.f, -1.f},
  };

TEST(bounding_sphere, ritter_ctor){
  auto sphere = bounding_volume::ritter_ctor(sample_vertices);
  EXPECT_EQ(sphere->get_sphere_radius(), 1.f);
  EXPECT_EQ(sphere->get_center_point(), Eigen::Vector3d(0.f, 1.f, 0.f));
}