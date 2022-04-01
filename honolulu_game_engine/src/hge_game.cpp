#include <hge_game.hpp>

namespace hnll {

HgeGame::HgeGame(const char* windowName) : upHve_m(std::make_unique<Hve>(windowName))
{
  loadData();
}

void HgeGame::runLoop()
{
  while (!glfwWindowShouldClose(glfwWindow_m))
  {
    processInput();
    updateGame();
    generateOutput();
  }
}

void HgeGame::processInput()
{

}

void HgeGame::updateGame()
{

}

void HgeGame::generateOutput()
{

}

void HgeGame::loadData()
{

}

} // namespace hnll