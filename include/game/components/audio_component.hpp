#pragma once

// hnll
#include <game/component.hpp>
#include <audio/audio_data.hpp>
#include <audio/engine.hpp>

namespace hnll::game {

class audio_component : public game::component
{
  public:
    static u_ptr<audio_component> create()
    {
      auto audio_comp = std::make_unique<audio_component>();
      return audio_comp;
    }
    audio_component() : game::component() {}

    void play_sound()
    {
      auto id = audio_data_->get_source_id();
      audio::engine::play_audio_from_source(id);
    }
    void get_ready_to_play()
    {
      if (!audio_data_->is_bound_to_buffer())
        audio::engine::bind_audio_to_buffer(*audio_data_);
      if (!audio_data_->is_bound_to_source())
        audio::engine::bind_buffer_to_source(*audio_data_);
    }
    // setter
    void set_raw_audio(const std::vector<ALshort>& raw_audio)
    { audio_data_->set_data(raw_audio); audio::engine::bind_audio_to_buffer(*audio_data_); }
    void set_raw_audio(std::vector<ALshort>&& raw_audio)
    { audio_data_->set_data(std::move(raw_audio)); audio::engine::bind_audio_to_buffer(*audio_data_); }

  private:
    u_ptr<audio::audio_data> audio_data_;
};

} // namespace hnll::game