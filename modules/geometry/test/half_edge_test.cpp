// hnll
#include <geometry/half_edge.hpp>

// lib
#include <gtest/gtest.h>

TEST(a, a){ EXPECT_TRUE(true); }

namespace hnll::geometry {

/*
 * - v0 - e2/ex - v2 - ex/ex - v4 - ex/ex - v0
 *    |         /  |         /  |         /
 *e0/ex  e1/e3 ex/e4  ex/ex ex/ex   ex/ex
 *    | /          |   /        | /
 * - v1 - ex/ex - v3 - ex/ex - v5 - ex/ex - v1
 */

TEST(geometry, half_edge) {

}
} // namespace hnll::geometry