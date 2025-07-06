#pragma once

#include "ThreadCommand.hpp"

#include <godot_cpp/variant/packed_byte_array.hpp>

namespace SK
{
class ThreadCommandUpdateTexture : public ThreadCommand
{
public:
    ThreadCommandUpdateTexture(godot::PackedByteArray pixelData, bool flipY);
    ~ThreadCommandUpdateTexture() override = default;

    void Execute() override;

private:
    godot::PackedByteArray m_pixelData;
    bool m_flipY;
};
}
