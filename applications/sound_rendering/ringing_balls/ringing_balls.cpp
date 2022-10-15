// hnll
#include <game/components/audio_component.hpp>
#include <game/actor.hpp>
#include <game/engine.hpp>

namespace hnll {

class ringing_ball : public game::actor
{
  public:
    s_ptr<ringing_ball> create()
    {

    }
    ringing_ball()
    {

    }
  private:
    std::vector<short> create_sound(double force)
    {
      std::vector<short> raw_audio;
      float duration = force * max_duration_;
      float pitch    = force * max_pitch_;

      raw_audio.resize(static_cast<size_t>(frequency_ * duration));
      for (int i = 0; i < raw_audio.size(); i++) {
        raw_audio[i] = std::sin(pitch * M_PI * 2.0 * i / frequency_)
          * std::numeric_limits<ALshort>::max();
      }

      return raw_audio;
    }
    static uint32_t frequency_;
    static float    max_duration_;
    static float    max_pitch_;
    u_ptr<game::audio_component> audio_component_;
};

uint32_t ringing_ball::frequency_    = 44100;
float    ringing_ball::max_duration_ = 0.5f;
float    ringing_ball::max_pitch_    = 1000.f;

class app : public game::engine
{
  public:
    app() {
    }
  private:
    void setup_default_sound()
    {

    }
};

int main()
{
  app app{};
  try { app.run(); }
  catch (const std::exception& e){
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

} // namespace hnll