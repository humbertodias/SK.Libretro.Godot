#include "Video.hpp"

#include <SDL3/SDL_opengl.h>

#include "Libretro.hpp"
#include "Debug.hpp"

#include <gfx/scaler/pixconv.h>

#include <chrono>

using namespace godot;

namespace SK
{
void Video::RefreshCallback(const void* data, uint32_t width, uint32_t height, size_t pitch)
{
    if (!data || width == 0 || height == 0)
        return;

    auto instance = Libretro::GetInstance();
    if (!instance)
    {
        LogError("RefreshCallback: Null Instance.");
        return;
    }

    PackedByteArray pixel_data;

    if (data == RETRO_HW_FRAME_BUFFER_VALID)
    {
        int bytes_per_pixel = 4;

        pixel_data.resize(width * height * bytes_per_pixel);
        glReadPixels(0, 0, (int)width, (int)height, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data.ptrw());
        SDL_GL_SwapWindow(instance->m_video->m_sdl_window);

        if (instance->m_video->m_image.is_null() || instance->m_video->m_image_format != godot::Image::FORMAT_RGBA8 || width != instance->m_video->m_last_width || height != instance->m_video->m_last_height)
        {
            instance->m_video->m_last_width  = width;
            instance->m_video->m_last_height = height;
            instance->CreateTexture(godot::Image::FORMAT_RGBA8, pixel_data, (int32_t)width, (int32_t)height, true);
        }
        else
            instance->UpdateTexture(pixel_data, true);

        return;
    }

    switch (instance->m_video->m_pixel_format)
    {
    case RETRO_PIXEL_FORMAT_XRGB8888:
    {
        pixel_data.resize(width * height * 4);

        const uint8_t* src = static_cast<const uint8_t*>(data);
        uint8_t* dst = pixel_data.ptrw();

        conv_argb8888_abgr8888(dst, src, width, height, width * 4, pitch);

        if (instance->m_video->m_image.is_null() || instance->m_video->m_image_format != godot::Image::FORMAT_RGBA8 || width != instance->m_video->m_last_width || height != instance->m_video->m_last_height)
        {
            instance->m_video->m_last_width  = width;
            instance->m_video->m_last_height = height;
            instance->CreateTexture(godot::Image::FORMAT_RGBA8, pixel_data, (int32_t)width, (int32_t)height, false);
        }
        else
            instance->UpdateTexture(pixel_data, false);
    }
    break;
    case RETRO_PIXEL_FORMAT_RGB565:
    {
        pixel_data.resize(width * height * 4);

        const uint16_t* src = static_cast<const uint16_t*>(data);
        uint8_t* dst = pixel_data.ptrw();

        conv_rgb565_abgr8888(dst, src, width, height, width * 4, pitch);

        if (instance->m_video->m_image.is_null() || instance->m_video->m_image_format != Image::FORMAT_RGBA8 || width != instance->m_video->m_last_width || height != instance->m_video->m_last_height)
        {
            instance->m_video->m_last_width  = width;
            instance->m_video->m_last_height = height;
            instance->CreateTexture(Image::FORMAT_RGBA8, pixel_data, (int32_t)width, (int32_t)height, false);
        }
        else
            instance->UpdateTexture(pixel_data, false);
    }
    break;
    case RETRO_PIXEL_FORMAT_0RGB1555:
    {
        pixel_data.resize(width * height * 4);

        const uint16_t* src = static_cast<const uint16_t*>(data);
        uint8_t* dst = pixel_data.ptrw();

        conv_0rgb1555_argb8888(dst, src, width, height, width * 4, pitch);

        if (instance->m_video->m_image.is_null() || instance->m_video->m_image_format != godot::Image::FORMAT_RGBA8 || width != instance->m_video->m_last_width || height != instance->m_video->m_last_height)
        {
            instance->m_video->m_last_width  = width;
            instance->m_video->m_last_height = height;
            instance->CreateTexture(godot::Image::FORMAT_RGBA8, pixel_data, (int32_t)width, (int32_t)height, false);
        }
        else
            instance->UpdateTexture(pixel_data, false);
    }
    break;
    case RETRO_PIXEL_FORMAT_UNKNOWN:
    default:
        LogError("Unhandled pixel format: " + std::to_string(instance->m_video->m_pixel_format));
        return;
    }
}

uintptr_t Video::HwRenderGetCurrentFramebuffer()
{
    return 0;
}

retro_proc_address_t Video::HwRenderGetProcAddress(const char* sym)
{
    return reinterpret_cast<retro_proc_address_t>(SDL_GL_GetProcAddress(sym));
}
}
