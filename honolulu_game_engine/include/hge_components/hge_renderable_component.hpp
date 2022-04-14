#pragma once

#include <hge_component.hpp>
#include <utility.hpp>

// std
#include <memory>

namespace hnll {

enum class RenderType 
{
  SIMPLE,
  POINT_LIGHT,
  NEON_LIGHT
};

class HgeRenderableComponent : public HgeComponent
{
  public:

    using id_t = unsigned int;

    HgeRenderableComponent(id_t id, RenderType type) : id_m(id), renderType_m(type) {}
    ~HgeRenderableComponent() {}

    HgeRenderableComponent(const HgeRenderableComponent &) = delete;
    HgeRenderableComponent& operator=(const HgeRenderableComponent &) = delete;
    HgeRenderableComponent(HgeRenderableComponent &&) = default;
    HgeRenderableComponent& operator=(HgeRenderableComponent &&) = default;

    inline Transform& getTransform() { return *upTransform_m; }
    template<class V> void setTranslation(V&& vec) 
    { upTransform_m->translation_m = std::forward<V>(vec); }    
    template<class V> void setScale(V&& vec) 
    { upTransform_m->scale_m = std::forward<V>(vec); }
    template<class V> void setRotation(V&& vec) 
    { upTransform_m->rotation_m = std::forward<V>(vec); }    

    virtual void updateComponent(float dt) override {}

    id_t getId() const { return id_m; }

    const RenderType getRenderType() const { return renderType_m; }
  protected:
    // same as ownwer's
    id_t id_m;
    // update this member
    u_ptr<Transform> upTransform_m = std::make_unique<Transform>();
    RenderType renderType_m;
};

}