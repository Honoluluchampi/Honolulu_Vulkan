#pragma once

//std
#include <vector>
#include <memory>

namespace hnll {

class HgeGame
{
public:
  HgeGame();
  ~HgeGame();
  // delete copy ctor
  HgeGame(const HgeGame &) = delete;
  HgeGame& operator=(const HgeGame &) = delete;

  bool initialize();
  void runLoop();

  void addActor(const class HveActor& actor);
  void removeActor(const class HveActor& actor);

private:
  void processInput();
  void updateGame();
  void generateOutput();

  void loadData();
  void unLoadData();

  std::vector<std::unique_ptr<class HgeActor>> activeActors_m;
  std::vector<std::unique_ptr<class HgeActor>> pendingActors_m;

  bool isUpdating_m = false; // for update
  bool isRunning_m = false; // for run loop
};

} // namespace hnll