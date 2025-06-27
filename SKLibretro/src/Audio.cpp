#include "Audio.hpp"

#include <godot_cpp/variant/vector2.hpp>

#include "Libretro.hpp"
#include "Debug.hpp"

using namespace godot;

namespace SK
{
void Audio::SampleCallback(int16_t left, int16_t right)
{
    auto instance = Libretro::GetInstance();
    if (!instance)
    {
        LogError("SampleCallback: Null Instance.");
        return;
    }

    if (!instance->m_audio->m_audio_stream_generator_playback.is_valid())
        return;

    float l = left / 32768.0f;
    float r = right / 32768.0f;
    instance->m_audio->m_audio_stream_generator_playback->push_frame(Vector2(l, r));
}

size_t Audio::SampleBatchCallback(const int16_t* data, size_t frames)
{
    if (!data)
        return frames;

    auto instance = Libretro::GetInstance();
    if (!instance)
    {
        LogError("SampleBatchCallback: Null Instance.");
        return frames;
    }

    if (instance->m_audio->m_audio_stream_generator_playback.is_null())
        return frames;

    auto available_frames = instance->m_audio->m_audio_stream_generator_playback->get_frames_available();

    if (available_frames > instance->m_audio->m_audio_buffer_total_frames)
        instance->m_audio->m_audio_buffer_total_frames = available_frames;

    auto total_frames = instance->m_audio->m_audio_buffer_total_frames;
    uint32_t occupancy_percent = 0;
    if (total_frames > 0) {
        occupancy_percent = static_cast<uint32_t>(100.0f * (1.0f - static_cast<float>(available_frames) / static_cast<float>(total_frames)));
        if (occupancy_percent > 100)
            occupancy_percent = 100;
    }

    instance->m_audio->m_audio_buffer_occupancy = occupancy_percent;

    for (size_t i = 0; i < frames; ++i)
    {
        float l = data[i * 2]     / 32768.0f;
        float r = data[i * 2 + 1] / 32768.0f;
        instance->m_audio->m_audio_stream_generator_playback->push_frame(Vector2(l, r));
    }

    return frames;
}
}
