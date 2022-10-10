// hnll
#include <game/engine.hpp>
#include <geometry/mesh_separation.hpp>
#include <geome

namespace hnll {

class view_frustum_culling : public game::engine
{
  public:
    view_frustum_culling() : game::engine("view_frustum_culling")
    {

    }
  private:
    void add_separated_model(const std::string& filename)
    {

    }
};

}

int main()
{
  hnll::view_frustum_culling app{};
  try { app.run(); }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}