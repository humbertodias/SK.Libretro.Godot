#include "ThreadCommandUpdateTexture.hpp"

#include "Wrapper.hpp"

using namespace godot;

namespace SK
{
ThreadCommandUpdateTexture::ThreadCommandUpdateTexture(PackedByteArray pixelData, bool flipY)
: m_pixelData(pixelData)
, m_flipY(flipY)
{
}

void ThreadCommandUpdateTexture::Execute()
{
    Wrapper::GetInstance()->m_video_handler->UpdateTexture(m_pixelData, m_flipY);
}
}
