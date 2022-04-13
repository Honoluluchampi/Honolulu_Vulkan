#pragma once

namespace hnll {

class HgeComponent
{
public:
  HgeComponent(){}
  virtual ~HgeComponent(){}

  inline void update(float dt) 
  { updateComponent(dt); }
private:
  virtual void updateComponent(float dt) {}
};

} // namespace hnll