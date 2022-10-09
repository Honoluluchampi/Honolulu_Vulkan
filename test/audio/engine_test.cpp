// hnll
#include <audio/engine.hpp>
#include <audio/audio_data.hpp>

// gtest
#include <gtest/gtest.h>

#define eq EXPECT_EQ

using namespace hnll::audio;

TEST(hae, ctor)
{
  engine::start_hae_context();
    eq(engine::remaining_pending_sources_count(), SOURCE_COUNT);
}

TEST(hae, audio_process_functions)
{
  audio_data audio_data;
  eq(engine::bind_audio_to_buffer(audio_data), result::SUCCESS);
  eq(engine::bind_buffer_to_source(audio_data), result::SUCCESS);

  eq(engine::remaining_pending_sources_count(), SOURCE_COUNT - 1);
  eq(audio_data.is_bound_to_buffer(), true);
  eq(audio_data.is_bound_to_source(), true);

  eq(engine::remove_audio_from_source(audio_data), result::SUCCESS);
  eq(engine::remove_audio_resources(audio_data), result::SUCCESS);

  eq(engine::remaining_pending_sources_count(), SOURCE_COUNT);
  eq(audio_data.is_bound_to_buffer(), false);
  eq(audio_data.is_bound_to_source(), false);
}

TEST(hae, dtor)
{
  engine::kill_hae_context();
}