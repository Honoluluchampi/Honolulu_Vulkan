#pragma once

namespace hnll {

class HgeComponent
{
public:
  using compId = unsigned;

  HgeComponent();
  virtual ~HgeComponent(){}

  inline void update(float dt) 
  { updateComponent(dt); }

#ifndef __IMGUI_DISABLED
  virtual void updateImgui(){}
#endif

  compId getCompId() const { return id_m; }
private:
  virtual void updateComponent(float dt) {}
  compId id_m;
};

} // namespace hnll