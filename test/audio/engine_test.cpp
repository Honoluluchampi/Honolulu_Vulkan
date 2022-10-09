// hnll
#include <audio/engine.hpp>
#include <audio/audio_data.hpp>

// gtest
#include <gtest/gtest.h>

using namespace hnll::audio;

TEST(hae, ctor)
{
  engine::start_hae_context();
    EXPECT_EQ(engine::remaining_pending_sources_count(), SOURCE_COUNT);
}

TEST(hae, audio_process_functions)
{
  audio_data audio_data;
  EXPECT_EQ(engine::bind_audio_to_buffer(audio_data), result::SUCCESS);
  EXPECT_EQ(engine::bind_buffer_to_source(audio_data), result::SUCCESS);

  EXPECT_EQ(engine::remaining_pending_sources_count(), SOURCE_COUNT - 1);
  EXPECT_EQ(audio_data.is_bound_to_buffer(), true);
  EXPECT_EQ(audio_data.is_bound_to_source(), true);

  EXPECT_EQ(engine::remove_audio_from_source(audio_data), result::SUCCESS);
  EXPECT_EQ(engine::remove_audio_resources(audio_data), result::SUCCESS);

  EXPECT_EQ(engine::remaining_pending_sources_count(), SOURCE_COUNT);
  EXPECT_EQ(audio_data.is_bound_to_buffer(), false);
  EXPECT_EQ(audio_data.is_bound_to_source(), false);
}

TEST(hae, dtor)
{
  engine::kill_hae_context();
}