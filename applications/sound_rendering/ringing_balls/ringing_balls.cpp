// hnll
#include <game/components/audio_component.hpp>
#include <game/actor.hpp>
#include <game/engine.hpp>
#include <game/components/rigid_component.hpp>
#include <geometry/bounding_volume.hpp>
#include <game/components/mesh_component.hpp>
#include <game/components/point_light_component.hpp>
#include <game/actors/default_camera.hpp>
#include <physics/engine.hpp>
#include <physics/collision_info.hpp>
#include <physics/collision_detector.hpp>

using namespace hnll;

class rigid_ball : public hnll::game::actor
{
public:
    rigid_ball(const Eigen::Vector3d& center_point, double radius) : hnll::game::actor(){}

    static s_ptr<rigid_ball> create(const Eigen::Vector3d& center_point, double radius)
    {
        // create ball actor and its mesh
        auto ball = std::make_shared<rigid_ball>(center_point, radius);
        auto ball_mesh = hnll::game::engine::get_mesh_model_sp("sphere");
        auto ball_mesh_vertex_position_list = ball_mesh->get_vertex_position_list();
        auto ball_mesh_comp = hnll::game::mesh_component::create(ball, std::move(ball_mesh));

        // create bounding_sphere
        auto bounding_sphere = hnll::geometry::bounding_volume::create_bounding_sphere
                (hnll::geometry::bv_ctor_type::RITTER, ball_mesh_vertex_position_list);
        ball->rigid_component_ = game::rigid_component::create_from_bounding_volume(*ball, std::move(bounding_sphere));

        ball->position_ = glm::vec3{center_point.x(), center_point.y(), center_point.z()};
        ball->set_translation(glm::vec3{center_point.x(), center_point.y(), center_point.z()});
        ball->velocity_ = {0.f, 0.f, 0.f};
        // register the ball to the engine
        hnll::game::engine::add_actor(ball);
        physics::collision_detector::add_rigid_component(ball->rigid_component_);
        return ball;
    };

    void update_actor(float dt) override
    {
        position_ += velocity_ * dt;
        velocity_.y += gravity_ * dt;
        this->set_translation(position_);
    }

    // this update function is invoked if collision_detector detects collision with other component (plane in this situation)
    void re_update(const hnll::physics::collision_info& info) override
    {
        position_.y *= -1;
        velocity_.y = -velocity_.y * restitution_;
        this->set_translation(position_);
    }

private:
    glm::vec3 position_;
    glm::vec3 velocity_;
    double gravity_ = 40.f;
    double restitution_ = 0.5;
    s_ptr<hnll::game::rigid_component> rigid_component_;
};

// plate is bounding box of which thickness is 0.
class rigid_plane : public hnll::game::actor
{
public:
    rigid_plane() : actor(){}
    static s_ptr<rigid_plane> create()
    {
        auto plane = std::make_shared<rigid_plane>();
        auto plane_mesh = hnll::game::engine::get_mesh_model_sp("plane");
        auto plane_mesh_vertices = plane_mesh->get_vertex_position_list();
        auto plane_mesh_comp = hnll::game::mesh_component::create(plane, std::move(plane_mesh));
        auto bounding_box = hnll::geometry::bounding_volume::create_aabb(plane_mesh_vertices);

        plane->rigid_component_ = game::rigid_component::create_from_bounding_volume(*plane, std::move(bounding_box));

        plane->set_translation({0.f, 1.f, 0.f});
        hnll::game::engine::add_actor(plane);
        physics::collision_detector::add_rigid_component(plane->rigid_component_);

        return plane;
    }
private:
    s_ptr<game::rigid_component> rigid_component_;
    glm::vec3 position_;
};

class falling_ball_app : public hnll::game::engine
{
public:
    falling_ball_app() : hnll::game::engine("falling ball")
    {
        // set camera position
        camera_up_->set_translation(glm::vec3{0.f, 0.f, -20.f});
        // add light
        auto light = hnll::game::actor::create();
        auto light_component = hnll::game::point_light_component::create(light, 100.f);
        add_point_light(light, light_component);
        light->set_translation({0.f, -20.f, 0.f});
        // add rigid ball
        auto ball = rigid_ball::create({0.f, -5.f, 0.f}, 1.f);
        // add plane
        auto rigid_plane = rigid_plane::create();
    }

    void update_game(float dt) override
    {

    }

private:
    hnll::physics::engine physics_engine_{};
};

int main()
{
    falling_ball_app app{};
    try { app.run(); }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}