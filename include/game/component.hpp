#pragma once

// std
#include <memory>

template <class T> using s_ptr = std::shared_ptr<T>;
template <class T> using u_ptr = std::unique_ptr<T>;

namespace hnll {
namespace game {

class component
{
public:
  using id = unsigned;

  component();
  virtual ~component(){}

  inline void update(float dt) { update_component(dt); }

#ifndef IMGUI_DISABLED
  virtual void update_gui(){}
#endif

  // getter
  id get_id() const { return id_; }
private:
  virtual void update_component(float dt) {}
  id id_;
};

} // namespace game
} // namespace hnll