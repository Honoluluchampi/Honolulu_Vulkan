#pragma once

namespace hnll {

class HgeComponent
{
public:
  using id_t = unsigned;

  HgeComponent();
  virtual ~HgeComponent(){}

  inline void update(float dt) 
  { updateComponent(dt); }

  id_t getCompId() const { return id_m; }
private:
  virtual void updateComponent(float dt) {}
  id_t id_m;
};

} // namespace hnll