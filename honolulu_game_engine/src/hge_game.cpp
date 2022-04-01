#include <hge_game.hpp>

namespace hnll {

HgeGame::HgeGame(const char* windowName) : pHve_m(std::make_unique<Hve>(windowName))
{
  loadData();
}

void HgeGame::runLoop()
{
  processInput();
  updateGame();
  generateOutput();
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

} // namespace hnll