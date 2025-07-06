#include "ThreadCommandInitAudio.hpp"

#include <godot_cpp/classes/audio_stream_player.hpp>

#include <mutex>

#include "Wrapper.hpp"

namespace SK
{
ThreadCommandInitAudio::ThreadCommandInitAudio(float bufferCapacitySec, double sampleRate)
: m_bufferCapacitySec(bufferCapacitySec)
, m_sampleRate(sampleRate)
{
}

void ThreadCommandInitAudio::Execute()
{
    auto instance = Wrapper::GetInstance();

    std::unique_lock<std::mutex> lock(instance->m_mutex);

    instance->m_audio_handler->Init(m_bufferCapacitySec, m_sampleRate);

    instance->m_mutex_done = true;
    instance->m_condition_variable.notify_one();
}
}
