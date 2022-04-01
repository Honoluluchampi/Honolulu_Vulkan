#include <hge_game.hpp>

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() 
{
  hnll::HgeGame app{"hello"};

  try {
    app.upHve_m->run();
  } 
  catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}