#pragma once

// std
#include <cassert>
#include <concepts>

// lib
#include <glm/gtc/matrix_transform.hpp>
#include <eigen3/Eigen/Core>

// glm concepts
template <class V>
concept glm_vec = requires(V& vec){ glm::normalize(vec); };
template <class M>
concept glm_mat = requires(M& mat){ { glm::transpose(mat) }; };

// eigen concepts (eigen vector is defined as matrix(n,1))
template <class M>
concept eigen_mat = requires(M& mat) { mat.outerSize(); };

void print_vec(const glm_vec auto& a, std::string name)
{
  std::cout << name << " :";
  for (int i = 0; i < a.length(); i++) std::cout << " " << a[i];
  std::cout << std::endl;
}
void print_mat(const glm_mat auto& a, std::string name)
{
  std::cout << name << " :" << std::endl;
  for (int i = 0; i < a.length(); i++) {
    for (int j = 0; j < a[0].length(); j++) {
      std::cout << a[i][j] << " ";
    }
    std::cout << std::endl;
  }
}

// glm equal
bool roughly_equal(const glm_vec auto& a, const glm_vec auto& b, double epsilon = 0.0001) {
  for (int i = 0; i < a.length(); i++)
    if (std::abs(a[i] - b[i]) > epsilon) {
      print_vec(a, "a");
      print_vec(b, "b");
      return false;
    }
  return true;
}
bool roughly_equal(const glm_mat auto& a, const glm_mat auto& b, double epsilon = 0.0001) {
    for (int i = 0; i < a.length(); i++)
      for (int j = 0; j < a[0].length(); j++)
       if (std::abs(a[i][j] - b[i][j]) > epsilon) {
         print_mat(a, "a");
         print_mat(b, "b");
         return false;
       }
    return true;
}

// eigen equal
bool roughly_equal(eigen_mat auto& a, eigen_mat auto& b, double epsilon = 0.0001) {
    for (int i = 0; i < a.innerSize(); i++) {
      // for mat
      if (a.outerSize() >= 2) {
        for (int j = 0; j < a.outerSize(); j++)
        if (std::abs(a(i, j) - b(i, j)) > epsilon) return false;
      }
      // for vec
      else {
        if (std::abs(a(i) - b(i)) > epsilon) return false;
      }
    }
    return true;
}