#pragma once

#include <hge_component.hpp>

namespace hnll {

// how to render
enum class RenderType
{
  SIMPLE,
  POINT_LIGHT
};

class HgeRenderableComponent : public HgeComponent
{
  public:
    using id_t = unsigned int;

    HgeRenderableComponent(id_t id, RenderType renderType) : id_m(id), renderType_m(renderType) {}
    ~HgeRenderableComponent() {}

    HgeRenderableComponent(const HgeRenderableComponent &) = delete;
    HgeRenderableComponent& operator=(const HgeRenderableComponent &) = delete;
    HgeRenderableComponent(HgeRenderableComponent &&) = default;
    HgeRenderableComponent& operator=(HgeRenderableComponent &&) = default;

    virtual void update(float dt) override {}

    id_t getId() const { return id_m; }
    RenderType getRenderType() const { return renderType_m; }

  protected:
    // same as ownwer's
    id_t id_m;
    RenderType renderType_m;
};

}