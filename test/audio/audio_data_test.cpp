// hnll
#include <audio/engine.hpp>
#include <audio/audio_data.hpp>

// gtest
#include <gtest/gtest.h>

using namespace hnll::audio;

TEST(audio_data, construct)
{
  audio_data audio_data;
    EXPECT_EQ(audio_data.get_sampling_rate(), 44100);
    EXPECT_EQ(audio_data.get_format(), AL_FORMAT_MONO16);
}

TEST(audio_data, setter_getter)
{
  audio_data data;
  data.set_data(std::vector<ALshort>{1000, 1000, 1000});
  data.set_sampling_rate(22050);
  data.set_format(AL_FORMAT_MONO8);

    EXPECT_EQ(*data.get_data(), 1000);
    EXPECT_EQ(data.get_sampling_rate(), 22050);
    EXPECT_EQ(data.get_format(), AL_FORMAT_MONO8);
  // ALshort is 16-bit
    EXPECT_EQ(data.get_data_size_in_byte(), 3 * 2);
}