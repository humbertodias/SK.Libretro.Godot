#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

#include <libretro.h>

namespace SK
{
class Input
{
public:
    static void PollCallback();
    static int16_t StateCallback(uint32_t port, uint32_t device, uint32_t index, uint32_t id);
    
    static bool RumbleInterfaceSetRumbleState(uint32_t port, retro_rumble_effect effect, uint16_t strength);

    struct RetroController
    {
        std::string name;
        uint32_t id;
    };

    // TODO: Handle this better when needed for UI
    struct RetroDevice
    {
        uint32_t port;
        uint32_t device;
        uint32_t index;
        uint32_t id;
        std::string description;
    };

    std::unordered_map<uint32_t, uint16_t> m_joypad_buttons;

    std::unordered_map<uint32_t, int16_t> m_mouse_x;
    std::unordered_map<uint32_t, int16_t> m_mouse_y;
    std::unordered_map<uint32_t, uint32_t> m_mouse_buttons;

    std::unordered_map<uint32_t, uint32_t> m_keyboard_keys;

    std::unordered_map<uint32_t, int16_t> m_lightgun_x;
    std::unordered_map<uint32_t, int16_t> m_lightgun_y;
    std::unordered_map<uint32_t, int16_t> m_lightgun_is_offscreen;
    std::unordered_map<uint32_t, uint32_t> m_lightgun_buttons;

    std::unordered_map<uint32_t, int16_t> m_pointer_x;
    std::unordered_map<uint32_t, int16_t> m_pointer_y;
    std::unordered_map<uint32_t, int16_t> m_pointer_pressed;
    std::unordered_map<uint32_t, int16_t> m_pointer_count;

    std::unordered_map<uint32_t, int16_t> m_analog_left_x;
    std::unordered_map<uint32_t, int16_t> m_analog_left_y;
    std::unordered_map<uint32_t, int16_t> m_analog_right_x;
    std::unordered_map<uint32_t, int16_t> m_analog_right_y;

    std::vector<std::vector<RetroController>> m_controllers;
    std::vector<RetroDevice> m_devices;
    retro_keyboard_event_t m_keyboard_event = nullptr;
};
}
