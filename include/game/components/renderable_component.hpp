#pragma once

// hnll
#include <game/component.hpp>
#include <utils/utils.hpp>

// std
#include <memory>

namespace hnll {

enum class RenderType 
{
  SIMPLE,
  POINT_LIGHT,
  LINE
};

// forward declaration
class HgeActor;
using actorId = unsigned int;

class HgeRenderableComponent : public HgeComponent
{
  public:
    HgeRenderableComponent(actorId id, RenderType type) : 
    HgeComponent(), id_m(id), renderType_m(type) {}
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

    actorId getActorId() const { return id_m; }

    const RenderType getRenderType() const { return renderType_m; }
  protected:
    // same as ownwer's
    actorId id_m;
    // update this member
    u_ptr<Transform> upTransform_m = std::make_unique<Transform>();
    RenderType renderType_m;
};

}