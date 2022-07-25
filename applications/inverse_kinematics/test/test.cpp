// hnll
#include <utils/utils.hpp>
#include <bone.hpp>
#include "test_utils.hpp"

// lib
#include <gtest/gtest.h>

template <typename T> constexpr T pi;
template <std::floating_point F> constexpr F pi<F> = static_cast<F>(3.141592653589793);

TEST(cyclic_coordinate_descent, rotate_mat)
{
  glm::vec3 point = { 0.f, 1.f, 0.f };
  hnll::utils::transform tf;

  // rotate pi/4 around z axis
  tf.rotation = { glm::vec3(0.f, 0.f, pi<float>/4.f) };
  glm::vec4 destination = tf.rotate_mat4() * glm::vec4{ point, 0.f};
  double r2 = std::sqrt(2);
  EXPECT_TRUE(roughly_equal(destination, glm::vec4{-r2/2.f, r2/2.f, 0.f, 0.f}));

  // rotate pi/2 around y axis then rotate
  tf.rotation = { glm::vec3(pi<float>/4.f, 0.f, -pi<float>/4.f) };
  glm::vec4 destination2 = tf.rotate_mat4() * glm::vec4{ point, 0.f };
  EXPECT_TRUE(roughly_equal(destination2, glm::vec4{r2/2.f, 1/2.f, 1/2.f, 0.f}));
}

TEST(cyclic_coordinate_descent, update_transform)
{
  auto pif = pi<float>;
  float rot_list[] = { 0.f, pif/4.f, pif/2.f, 3 * pif/4.f, pif, -pif/4.f, -pif/2.f -3 * pif/4.f };
  for (int i = 0; i < 8; i++) {
      for (int k = 0; k < 8; k++) {
        EXPECT_TRUE(test_update_transform(glm::vec3{rot_list[i], 0, rot_list[k]}));
      }
  }
}

TEST(cyclic_coordinate_descent_rotate, glm_rotate_check)
{
  auto glm_rotate = glm::rotate(glm::mat4{1.f}, pi<float>/4.f, glm::vec3{0.f, 1.f, 0.f});
  hnll::utils::transform tf{};
  tf.rotation = {0.f, pi<float>/4.f, 0.f};
  auto hnll_rotate = tf.rotate_mat4();
  EXPECT_TRUE(roughly_equal(glm_rotate, hnll_rotate));
}