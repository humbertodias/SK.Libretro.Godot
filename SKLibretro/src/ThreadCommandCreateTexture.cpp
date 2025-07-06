#include "ThreadCommandCreateTexture.hpp"

#include <godot_cpp/classes/image_texture.hpp>

#include "Wrapper.hpp"

using namespace godot;

namespace SK
{
ThreadCommandCreateTexture::ThreadCommandCreateTexture(Image::Format image_format, PackedByteArray pixel_data, int32_t width, int32_t height, bool flip_y)
: m_imageFormat(image_format)
, m_pixelData(pixel_data)
, m_width(width)
, m_height(height)
, m_flipY(flip_y)
{
}

void ThreadCommandCreateTexture::Execute()
{
    Wrapper::GetInstance()->m_video_handler->CreateTexture(m_width, m_height, m_imageFormat, m_pixelData, m_flipY);
}
}
