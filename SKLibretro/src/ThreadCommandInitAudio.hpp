#pragma once

#include "ThreadCommand.hpp"

namespace SK
{
class ThreadCommandInitAudio : public ThreadCommand
{
public:
    ThreadCommandInitAudio(float bufferCapacitySec, double sampleRate);
    ~ThreadCommandInitAudio() override = default;

    void Execute() override;

private:
    float m_bufferCapacitySec;
    double m_sampleRate;
};
}
