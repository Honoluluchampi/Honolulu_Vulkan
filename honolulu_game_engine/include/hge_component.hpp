#pragma once

namespace hnll {

class HgeComponent
{
public:
  HgeComponent(){}
  virtual ~HgeComponent(){}

private:
  virtual void update(float dt){}
};

} // namespace hnll