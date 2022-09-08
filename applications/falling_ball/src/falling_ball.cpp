#include <game/engine.hpp>

int main()
{
  hnll::game::engine app{"falling ball"};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}