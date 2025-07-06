#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/audio_stream_generator.hpp>
#include <godot_cpp/classes/audio_stream_generator_playback.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>

#include <cstdint>

#include <libretro.h>

namespace SK
{
class AudioHandler
{
public:
    static void SampleCallback(int16_t left, int16_t right);
    static size_t SampleBatchCallback(const int16_t* data, size_t frames);

    void Init(float buffer_capacity_sec, double sample_rate);
    void DeInit();

    void SetAudioBufferStatusCallback(retro_audio_buffer_status_callback_t callback);
    void CallAudioBufferStatusCallback();

private:
    godot::Ref<godot::AudioStreamGenerator> m_audio_stream_generator = nullptr;
    godot::Ref<godot::AudioStreamGeneratorPlayback> m_audio_stream_generator_playback = nullptr;
    godot::AudioStreamPlayer* m_audio_stream_player = nullptr;
    float m_audio_buffer_capacity_sec = 0;
    double m_audio_sample_rate = 0.0;
    uint32_t m_audio_buffer_total_frames = 0;
    uint32_t m_audio_buffer_occupancy = 0;
    retro_audio_buffer_status_callback_t m_audio_buffer_status_callback = nullptr;
};
}
