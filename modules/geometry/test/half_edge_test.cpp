// hnll
#include <geometry/half_edge.hpp>

// lib
#include <gtest/gtest.h>

namespace hnll::geometry {

/*
 * - v0 - e2/ex - v2 - ex/ex - v4 - ex/ex - v0
 *    |         /  |         /  |         /
 *e0/ex  e1/e3 ex/e4  ex/ex ex/ex   ex/ex
 *    | /          |   /        | /
 * - v1 - ex/ex - v3 - ex/ex - v5 - ex/ex - v1
 */

TEST(half_edge, pair) {
  auto model = mesh_model::create();
  auto v0 = vertex::create({ 0.f, 0.f, 0.f });
  auto v1 = vertex::create({ 0.f, -1.f, 0.f});
  auto v2 = vertex::create({ 1.f, 0.f, 0.f });
  auto v3 = vertex::create({ 1.f, -1.f, 0.f});
  auto v4 = vertex::create({ 0.f, 0.f, 1.f });
  auto v5 = vertex::create({ 0.f, -1.f, 1.f});

  model->add_face(v0, v1, v2);
  EXPECT_EQ(model->get_half_edge(v1, v2)->get_pair(), nullptr);
  model->add_face(v1, v3, v2);
  EXPECT_EQ(model->get_half_edge(v1, v2), model->get_half_edge(v1, v2)->get_pair()->get_pair());
  model->add_face(v2, v3, v4);
  model->add_face(v3, v5, v4);
  model->add_face(v4, v5, v0);
  model->add_face(v5, v1, v0);
  EXPECT_EQ(model->get_half_edge(v0, v1), model->get_half_edge(v0, v1)->get_pair()->get_pair());
  // invalid half-edge
  EXPECT_EQ(model->get_half_edge(v0, v3), nullptr);
  // unpaired half-edge
  EXPECT_EQ(model->get_half_edge(v0, v2)->get_pair(), nullptr);
}
} // namespace hnll::geometry