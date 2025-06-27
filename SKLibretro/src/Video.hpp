#pragma once

#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>

#include <cstdint>

#include <SDL3/SDL_video.h>

#include <libretro.h>

namespace SK
{
class Video
{
public:
    static void RefreshCallback(const void* data, uint32_t width, uint32_t height, size_t pitch);
    static uintptr_t HwRenderGetCurrentFramebuffer();
    static retro_proc_address_t HwRenderGetProcAddress(const char* sym);

    retro_pixel_format m_pixel_format = RETRO_PIXEL_FORMAT_UNKNOWN;
    uint32_t m_last_width = 0;
    uint32_t m_last_height = 0;
    godot::Image::Format m_image_format;
    godot::Ref<godot::Image> m_image = nullptr;
    godot::Ref<godot::ImageTexture> m_texture = nullptr;
    SDL_Window* m_sdl_window = nullptr;
    SDL_GLContext m_sdl_gl_context = nullptr;
    retro_hw_context_reset_t m_context_reset = nullptr;
    retro_hw_context_reset_t m_context_destroy = nullptr;
};
}
